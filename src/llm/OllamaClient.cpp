#include "langcad/llm/OllamaClient.hpp"

#include "langcad/api/ShapeJson.hpp"
#include "langcad/geometry/ShapeValidator.hpp"

#include <httplib.h>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <chrono>
#include <cctype>
#include <cstdlib>
#include <stdexcept>
#include <vector>
#include <utility>

namespace langcad::llm {

namespace {

std::string envOrDefault(const char* name, const std::string& fallback) {
    const char* value = std::getenv(name);
    return value ? std::string(value) : fallback;
}

std::string lowercase(const std::string& value) {
    std::string output;
    output.reserve(value.size());

    for (char character : value) {
        output.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(character))));
    }

    return output;
}

bool containsAny(const std::string& value, const std::vector<std::string>& needles) {
    for (const auto& needle : needles) {
        if (value.find(needle) != std::string::npos) {
            return true;
        }
    }

    return false;
}

int largestIntegerIn(const std::string& value) {
    int largest = 0;
    int current = 0;
    bool reading_number = false;

    for (char character : value) {
        if (std::isdigit(static_cast<unsigned char>(character))) {
            reading_number = true;
            current = current * 10 + (character - '0');
        } else if (reading_number) {
            largest = std::max(largest, current);
            current = 0;
            reading_number = false;
        }
    }

    if (reading_number) {
        largest = std::max(largest, current);
    }

    return largest;
}

geometry::ShapeDefinition rejectedDefinition() {
    geometry::ShapeDefinition definition;
    definition.shape_type = "faceted_shape";
    definition.units = "unitless";
    definition.centered = true;
    return definition;
}

geometry::ShapeDefinition invalidDefinitionForPrompt(const std::string& prompt) {
    std::string text = lowercase(prompt);

    if (containsAny(text, {
        "codigo",
        "c++",
        "/etc/passwd",
        "rm -rf",
        "command",
        "schema",
        "shell",
        "script",
        "file",
        "path",
        "archivo",
        "ruta",
        "multiples",
        "multiple",
        "varias figuras",
        "varios",
        "tres figuras"
    })) {
        return rejectedDefinition();
    }

    if (text.find("-") != std::string::npos && containsAny(text, {"radio", "radius", "altura", "height", "tamano", "size", "base"})) {
        return rejectedDefinition();
    }

    if (containsAny(text, {"segmentos", "segments"}) && largestIntegerIn(text) > 256) {
        geometry::ShapeDefinition definition;
        definition.shape_type = containsAny(text, {"esfera", "sphere"}) ? "sphere" : "cylinder";
        definition.dimensions["radius"] = 1.0;
        definition.units = "unitless";
        definition.tessellation.segments = largestIntegerIn(text);
        definition.centered = true;

        if (definition.shape_type == "cylinder") {
            definition.dimensions["height"] = 1.0;
        }

        return definition;
    }

    geometry::ShapeDefinition definition;
    return definition;
}

bool hasPromptGuardRejection(const geometry::ShapeDefinition& definition) {
    return !definition.shape_type.empty();
}

std::string buildPrompt(const std::string& user_prompt) {
    return
        "Convert the user's request into one JSON ShapeDefinition only. "
        "Do not include code, file paths, commands, markdown, or explanations. "
        "The JSON must contain shape_type, dimensions, vertices, faces, units, render, and centered. "
        "The only supported shape_type value for LLM output is faceted_shape. "
        "Use faceted_shape for every drawable solid, including cubes, pyramids, prisms, boxes, and other solids. "
        "If the user asks for code, commands, shell execution, files, paths, schema changes, or multiple shapes, "
        "return a ShapeDefinition that validation will reject: "
        "{\"shape_type\":\"faceted_shape\",\"dimensions\":{},\"vertices\":[],\"faces\":[],\"units\":\"unitless\",\"render\":{},\"centered\":true}. "
        "Use centered=true. Preserve every explicit numeric value from the user. "
        "Spanish terms: cubo=cube, cilindro=cylinder, piramide=pyramid, esfera=sphere, radio=radius, "
        "altura=height, tamano/size=size, base=base_size, segmentos=render.segments, anillos=render.rings, "
        "punto/vertice=vertex, vertices=vertices, arista=edge, aristas=edges, cara=face, caras=faces. "
        "For faceted_shape, dimensions must be {}, render must be {}, and include vertices and faces arrays. "
        "Each vertex must be an object with numeric x, y, and z. "
        "Each face must be an array of at least three integer vertex indices into vertices. "
        "Faces define the boundary loops of the solid. Do not include edges; the runtime derives edges from faces. "
        "Indices are zero-based and must reference existing vertices only. Do not invent extra fields. "
        "For a cube or box, provide exactly the eight corner vertices and six quad faces. Do not use a triangle fan for a cube. "
        "For a pyramid, provide the base vertices, apex vertex, one base face, and triangular side faces. "
        "For curved solids like cylinders or spheres, use the smallest faceted approximation that satisfies explicit user values. "
        "Example: 'cubo de tamano 2' becomes "
        "{\"shape_type\":\"faceted_shape\",\"dimensions\":{},"
        "\"vertices\":[{\"x\":-1,\"y\":-1,\"z\":-1},{\"x\":1,\"y\":-1,\"z\":-1},{\"x\":1,\"y\":1,\"z\":-1},{\"x\":-1,\"y\":1,\"z\":-1},{\"x\":-1,\"y\":-1,\"z\":1},{\"x\":1,\"y\":-1,\"z\":1},{\"x\":1,\"y\":1,\"z\":1},{\"x\":-1,\"y\":1,\"z\":1}],"
        "\"faces\":[[0,1,2,3],[4,7,6,5],[0,4,5,1],[1,5,6,2],[2,6,7,3],[3,7,4,0]],"
        "\"units\":\"unitless\",\"render\":{},\"centered\":true}. "
        "Example: 'cara triangular con vertices (0,0,0), (1,0,0), (0,1,0)' becomes "
        "{\"shape_type\":\"faceted_shape\",\"dimensions\":{},"
        "\"vertices\":[{\"x\":0,\"y\":0,\"z\":0},{\"x\":1,\"y\":0,\"z\":0},{\"x\":0,\"y\":1,\"z\":0}],"
        "\"faces\":[[0,1,2]],\"units\":\"unitless\",\"render\":{},\"centered\":true}.\n\n"
        "User request: " + user_prompt;
}

} // namespace

OllamaClient::OllamaClient(OllamaConfig config)
    : config_(std::move(config))
{
}

OllamaShapeResponse OllamaClient::generateShapeDefinition(const std::string& prompt) const {
    OllamaShapeResponse output;
    output.model = config_.model;

    auto start = std::chrono::steady_clock::now();
    auto setLatency = [&output, start]() {
        auto end = std::chrono::steady_clock::now();
        output.latency_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    };

    geometry::ShapeDefinition guarded_definition = invalidDefinitionForPrompt(prompt);
    if (hasPromptGuardRejection(guarded_definition)) {
        output.validation = geometry::validateShapeDefinition(guarded_definition);
        output.shape_definition = output.validation.normalized_definition;
        output.ok = false;
        setLatency();
        return output;
    }

    try {
        httplib::Client client(config_.base_url);
        client.set_read_timeout(120, 0);
        client.set_write_timeout(30, 0);

        nlohmann::json request = {
            {"model", config_.model},
            {"prompt", buildPrompt(prompt)},
            {"stream", false},
            {"format", langcad::api::shapeDefinitionSchemaJson()},
            {"options", {{"temperature", 0}}}
        };

        auto response = client.Post("/api/generate", request.dump(), "application/json");

        if (!response) {
            output.error = "Ollama request failed";
            setLatency();
            return output;
        }

        if (response->status < 200 || response->status >= 300) {
            output.error = "Ollama returned HTTP status " + std::to_string(response->status);
            setLatency();
            return output;
        }

        nlohmann::json response_json = nlohmann::json::parse(response->body);

        if (!response_json.contains("response") || !response_json.at("response").is_string()) {
            output.error = "Ollama response did not contain a string field named 'response'";
            setLatency();
            return output;
        }

        nlohmann::json shape_json = nlohmann::json::parse(response_json.at("response").get<std::string>());
        auto parsed = langcad::api::parseShapeDefinitionJson(shape_json);

        output.validation = geometry::validateShapeDefinition(parsed.definition);
        output.validation.errors.insert(
            output.validation.errors.begin(),
            parsed.errors.begin(),
            parsed.errors.end()
        );
        output.validation.valid = output.validation.errors.empty();
        output.shape_definition = output.validation.normalized_definition;
        output.ok = output.validation.valid;
    } catch (const std::exception& error) {
        output.error = error.what();
    }

    setLatency();
    return output;
}

OllamaConfig ollamaConfigFromEnvironment() {
    OllamaConfig config;
    config.base_url = envOrDefault("OLLAMA_URL", config.base_url);
    config.model = envOrDefault("OLLAMA_MODEL", config.model);
    return config;
}

} // namespace langcad::llm
