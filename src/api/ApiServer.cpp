#include "langcad/api/ApiServer.hpp"

#include "langcad/api/ShapeJson.hpp"
#include "langcad/api/Trace.hpp"
#include "langcad/geometry/ShapeFactory.hpp"
#include "langcad/geometry/ShapeValidator.hpp"
#include "langcad/render/Camera.hpp"
#include "langcad/render/Renderer.hpp"
#include "langcad/scene/Scene.hpp"

#include <nlohmann/json.hpp>

#include <atomic>
#include <chrono>
#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <utility>

namespace langcad::api {

namespace {

struct ValidatedRequest {
    geometry::ShapeValidationResult validation;
    std::string shape_type;
    nlohmann::json dimensions = nlohmann::json::object();
};

long long elapsedMs(std::chrono::steady_clock::time_point start) {
    auto end = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

void addCorsHeaders(httplib::Response& response) {
    response.set_header("Access-Control-Allow-Origin", "*");
    response.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    response.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
    response.set_header("Access-Control-Max-Age", "86400");
}

void sendJson(httplib::Response& response, int status, const nlohmann::json& body) {
    addCorsHeaders(response);
    response.status = status;
    response.set_content(body.dump(), "application/json");
}

std::string errorTypeForValidation(const geometry::ShapeValidationResult& validation) {
    if (validation.valid) {
        return "";
    }

    for (const auto& error : validation.errors) {
        if (error.find("Invalid JSON") != std::string::npos) {
            return "invalid_json";
        }

        if (error.find("Dangerous field") != std::string::npos) {
            return "dangerous_field";
        }

        if (error.find("Unsupported field") != std::string::npos) {
            return "schema_violation";
        }
    }

    return "validation_failed";
}

std::string errorTypeForLlmResponse(const langcad::llm::OllamaShapeResponse& response) {
    if (response.ok) {
        return "";
    }

    if (!response.error.empty()) {
        return "llm_error";
    }

    return errorTypeForValidation(response.validation);
}

ValidatedRequest validateBody(const std::string& body) {
    ValidatedRequest request;

    try {
        nlohmann::json input = nlohmann::json::parse(body);
        auto parsed = parseShapeDefinitionJson(input);
        request.shape_type = parsed.definition.shape_type;
        request.dimensions = parsed.definition.dimensions;

        request.validation = geometry::validateShapeDefinition(parsed.definition);
        request.validation.errors.insert(
            request.validation.errors.begin(),
            parsed.errors.begin(),
            parsed.errors.end()
        );
        request.validation.valid = request.validation.errors.empty();
        request.shape_type = request.validation.normalized_definition.shape_type;
        request.dimensions = request.validation.normalized_definition.dimensions;
    } catch (const std::exception& error) {
        request.validation.valid = false;
        request.validation.errors.push_back(std::string("Invalid JSON: ") + error.what());
    }

    return request;
}

void renderSingleShape(
    std::unique_ptr<geometry::Shape3D> shape,
    const std::shared_ptr<std::atomic_bool>& stop_requested
) {
    langcad::scene::Scene scene;
    scene.add(std::move(shape));

    langcad::render::Renderer renderer(1024, 768, "MiniCAD - API Render");
    langcad::render::Camera camera;

    bool running = true;

    while (running && !stop_requested->load()) {
        running = renderer.handleEvents(camera);
        renderer.render(camera, scene);
    }
}

nlohmann::json requestPromptJson(const httplib::Request& request, std::vector<std::string>& errors) {
    try {
        nlohmann::json input = nlohmann::json::parse(request.body);

        if (!input.is_object()) {
            errors.push_back("Request body must be a JSON object");
            return {};
        }

        for (auto it = input.begin(); it != input.end(); ++it) {
            if (it.key() != "prompt") {
                errors.push_back("Unsupported field '" + it.key() + "'");
            }
        }

        if (!input.contains("prompt")) {
            errors.push_back("Missing required field 'prompt'");
            return input;
        }

        if (!input.at("prompt").is_string()) {
            errors.push_back("Field 'prompt' must be a string");
        }

        return input;
    } catch (const std::exception& error) {
        errors.push_back(std::string("Invalid JSON: ") + error.what());
        return {};
    }
}

} // namespace

ApiServer::ApiServer(langcad::llm::OllamaConfig ollama_config)
    : ollama_config_(std::move(ollama_config)),
      stopping_(false)
{
    registerRoutes();
}

ApiServer::~ApiServer() {
    {
        std::lock_guard<std::mutex> lock(render_mutex_);
        stopping_ = true;
    }

    render_cv_.notify_all();
    server_.stop();
    stopActiveRender();
}

void ApiServer::listen(const char* host, int port) {
    if (!server_.bind_to_port(host, port)) {
        throw std::runtime_error("Could not bind API server to requested host and port");
    }

    std::thread server_thread([this]() {
        server_.listen_after_bind();
    });

    runRenderLoopOnMainThread();

    server_.stop();

    if (server_thread.joinable()) {
        server_thread.join();
    }
}

void ApiServer::launchRender(std::unique_ptr<geometry::Shape3D> shape) {
    std::lock_guard<std::mutex> lock(render_mutex_);

    if (render_stop_) {
        render_stop_->store(true);
    }

    pending_render_ = std::move(shape);
    render_cv_.notify_one();
}

void ApiServer::stopActiveRender() {
    std::lock_guard<std::mutex> lock(render_mutex_);

    if (render_stop_) {
        render_stop_->store(true);
    }

    pending_render_.reset();
    render_stop_.reset();
    render_cv_.notify_all();
}

void ApiServer::runRenderLoopOnMainThread() {
    while (true) {
        std::unique_ptr<geometry::Shape3D> shape;
        std::shared_ptr<std::atomic_bool> stop_requested;

        {
            std::unique_lock<std::mutex> lock(render_mutex_);
            render_cv_.wait(lock, [this]() {
                return stopping_ || pending_render_ != nullptr;
            });

            if (stopping_) {
                break;
            }

            shape = std::move(pending_render_);
            stop_requested = std::make_shared<std::atomic_bool>(false);
            render_stop_ = stop_requested;
        }

        try {
            renderSingleShape(std::move(shape), stop_requested);
        } catch (const std::exception& error) {
            nlohmann::json log = {
                {"endpoint", "render_main_thread"},
                {"status", "error"},
                {"error", error.what()}
            };
            std::cerr << log.dump() << '\n';
        }

        {
            std::lock_guard<std::mutex> lock(render_mutex_);

            if (render_stop_ == stop_requested) {
                render_stop_.reset();
            }
        }
    }
}

void ApiServer::registerRoutes() {
    server_.Options(R"(.*)", [](const httplib::Request&, httplib::Response& response) {
        addCorsHeaders(response);
        response.status = 204;
    });

    server_.Get("/api/v1/health", [this](const httplib::Request&, httplib::Response& response) {
        auto start = std::chrono::steady_clock::now();
        std::string trace_id = makeTraceId();

        nlohmann::json body = {
            {"trace_id", trace_id},
            {"status", "ok"}
        };

        logger_.logRequest(trace_id, "/api/v1/health", "", "", nlohmann::json::object(), true, false, "", elapsedMs(start), "ok");
        sendJson(response, 200, body);
    });

    server_.Get("/api/v1/shapes", [this](const httplib::Request&, httplib::Response& response) {
        auto start = std::chrono::steady_clock::now();
        std::string trace_id = makeTraceId();

        nlohmann::json body = {
            {"trace_id", trace_id},
            {"status", "ok"},
            {"shapes", supportedShapesToJson()}
        };

        logger_.logRequest(trace_id, "/api/v1/shapes", "", "", nlohmann::json::object(), true, false, "", elapsedMs(start), "ok");
        sendJson(response, 200, body);
    });

    server_.Post("/api/v1/shapes/validate", [this](const httplib::Request& request, httplib::Response& response) {
        auto start = std::chrono::steady_clock::now();
        std::string trace_id = makeTraceId();
        auto validated = validateBody(request.body);

        nlohmann::json body = validationToJson(validated.validation);
        body["trace_id"] = trace_id;

        std::string status = validated.validation.valid ? "ok" : "invalid";
        logger_.logRequest(
            trace_id,
            "/api/v1/shapes/validate",
            "",
            validated.shape_type,
            validated.dimensions,
            validated.validation.valid,
            false,
            errorTypeForValidation(validated.validation),
            elapsedMs(start),
            status
        );
        sendJson(response, 200, body);
    });

    server_.Post("/api/v1/shapes/mesh", [this](const httplib::Request& request, httplib::Response& response) {
        auto start = std::chrono::steady_clock::now();
        std::string trace_id = makeTraceId();
        auto validated = validateBody(request.body);

        if (!validated.validation.valid) {
            nlohmann::json body = validationToJson(validated.validation);
            body["trace_id"] = trace_id;

            logger_.logRequest(trace_id, "/api/v1/shapes/mesh", "", validated.shape_type, validated.dimensions, false, false, errorTypeForValidation(validated.validation), elapsedMs(start), "invalid");
            sendJson(response, 400, body);
            return;
        }

        auto shape = geometry::createShapeFromDefinition(validated.validation.normalized_definition);
        auto mesh = shape->toMesh();

        nlohmann::json body = {
            {"trace_id", trace_id},
            {"status", "ok"},
            {"shape_type", validated.validation.normalized_definition.shape_type},
            {"mesh", meshToJson(mesh)}
        };

        logger_.logRequest(trace_id, "/api/v1/shapes/mesh", "", validated.shape_type, validated.dimensions, true, false, "", elapsedMs(start), "ok");
        sendJson(response, 200, body);
    });

    server_.Post("/api/v1/shapes/render", [this](const httplib::Request& request, httplib::Response& response) {
        auto start = std::chrono::steady_clock::now();
        std::string trace_id = makeTraceId();
        auto validated = validateBody(request.body);

        if (!validated.validation.valid) {
            nlohmann::json body = validationToJson(validated.validation);
            body["trace_id"] = trace_id;

            logger_.logRequest(trace_id, "/api/v1/shapes/render", "", validated.shape_type, validated.dimensions, false, false, errorTypeForValidation(validated.validation), elapsedMs(start), "invalid");
            sendJson(response, 400, body);
            return;
        }

        auto shape = geometry::createShapeFromDefinition(validated.validation.normalized_definition);
        launchRender(std::move(shape));

        nlohmann::json body = {
            {"trace_id", trace_id},
            {"status", "render_started"},
            {"shape_type", validated.validation.normalized_definition.shape_type},
            {"persistent", true},
            {"replaces_previous_render", true}
        };

        logger_.logRequest(trace_id, "/api/v1/shapes/render", "", validated.shape_type, validated.dimensions, true, true, "", elapsedMs(start), "render_started");
        sendJson(response, 202, body);
    });

    server_.Post("/api/v1/llm/shape-definition", [this](const httplib::Request& request, httplib::Response& response) {
        auto start = std::chrono::steady_clock::now();
        std::string trace_id = makeTraceId();
        std::vector<std::string> errors;
        nlohmann::json input = requestPromptJson(request, errors);

        if (!errors.empty()) {
            nlohmann::json body = {
                {"trace_id", trace_id},
                {"shape_definition", nullptr},
                {"validation", {{"valid", false}, {"errors", errors}}},
                {"model", ollama_config_.model},
                {"latency_ms", elapsedMs(start)}
            };

            logger_.logRequest(trace_id, "/api/v1/llm/shape-definition", ollama_config_.model, "", nlohmann::json::object(), false, false, "invalid_request", elapsedMs(start), "invalid_request");
            sendJson(response, 400, body);
            return;
        }

        langcad::llm::OllamaClient client(ollama_config_);
        auto llm_response = client.generateShapeDefinition(input.at("prompt").get<std::string>());

        nlohmann::json validation = validationToJson(llm_response.validation);
        if (!llm_response.error.empty()) {
            validation["valid"] = false;
            validation["errors"].push_back(llm_response.error);
        }

        nlohmann::json body = {
            {"trace_id", trace_id},
            {"shape_definition", llm_response.ok ? shapeDefinitionToJson(llm_response.shape_definition) : nlohmann::json(nullptr)},
            {"validation", validation},
            {"model", llm_response.model},
            {"latency_ms", llm_response.latency_ms}
        };

        std::string shape_type = llm_response.shape_definition.shape_type;
        nlohmann::json dimensions = llm_response.shape_definition.dimensions;
        std::string status = llm_response.ok ? "ok" : "invalid";

        logger_.logRequest(trace_id, "/api/v1/llm/shape-definition", llm_response.model, shape_type, dimensions, llm_response.ok, false, errorTypeForLlmResponse(llm_response), elapsedMs(start), status);
        sendJson(response, llm_response.ok ? 200 : 400, body);
    });

    server_.Post("/api/v1/llm/render", [this](const httplib::Request& request, httplib::Response& response) {
        auto start = std::chrono::steady_clock::now();
        std::string trace_id = makeTraceId();
        std::vector<std::string> errors;
        nlohmann::json input = requestPromptJson(request, errors);

        if (!errors.empty()) {
            nlohmann::json body = {
                {"trace_id", trace_id},
                {"status", "invalid_request"},
                {"validation", {{"valid", false}, {"errors", errors}}},
                {"model", ollama_config_.model}
            };

            logger_.logRequest(trace_id, "/api/v1/llm/render", ollama_config_.model, "", nlohmann::json::object(), false, false, "invalid_request", elapsedMs(start), "invalid_request");
            sendJson(response, 400, body);
            return;
        }

        langcad::llm::OllamaClient client(ollama_config_);
        auto llm_response = client.generateShapeDefinition(input.at("prompt").get<std::string>());

        nlohmann::json validation = validationToJson(llm_response.validation);
        if (!llm_response.error.empty()) {
            validation["valid"] = false;
            validation["errors"].push_back(llm_response.error);
        }

        if (!llm_response.ok) {
            nlohmann::json body = {
                {"trace_id", trace_id},
                {"status", "rejected"},
                {"render_started", false},
                {"shape_definition", nullptr},
                {"validation", validation},
                {"model", llm_response.model},
                {"latency_ms", llm_response.latency_ms}
            };

            logger_.logRequest(trace_id, "/api/v1/llm/render", llm_response.model, llm_response.shape_definition.shape_type, llm_response.shape_definition.dimensions, false, false, errorTypeForLlmResponse(llm_response), elapsedMs(start), "rejected");
            sendJson(response, 400, body);
            return;
        }

        auto shape = geometry::createShapeFromDefinition(llm_response.shape_definition);
        launchRender(std::move(shape));

        nlohmann::json body = {
            {"trace_id", trace_id},
            {"status", "render_started"},
            {"shape_type", llm_response.shape_definition.shape_type},
            {"persistent", true},
            {"replaces_previous_render", true},
            {"shape_definition", shapeDefinitionToJson(llm_response.shape_definition)},
            {"validation", validation},
            {"model", llm_response.model},
            {"latency_ms", llm_response.latency_ms}
        };

        logger_.logRequest(trace_id, "/api/v1/llm/render", llm_response.model, llm_response.shape_definition.shape_type, llm_response.shape_definition.dimensions, true, true, "", elapsedMs(start), "render_started");
        sendJson(response, 202, body);
    });
}

} // namespace langcad::api
