#include "langcad/api/ShapeJson.hpp"

#include <algorithm>
#include <array>

namespace langcad::api {

namespace {

bool isAllowed(const std::string& key, const std::vector<std::string>& allowed) {
    return std::find(allowed.begin(), allowed.end(), key) != allowed.end();
}

bool isDangerousKey(const std::string& key) {
    static const std::vector<std::string> dangerous_keys = {
        "command",
        "code",
        "path",
        "shell",
        "script",
        "file"
    };

    return std::find(dangerous_keys.begin(), dangerous_keys.end(), key) != dangerous_keys.end();
}

void rejectDangerousKeysRecursive(
    const nlohmann::json& value,
    const std::string& scope,
    std::vector<std::string>& errors
) {
    if (value.is_object()) {
        for (auto it = value.begin(); it != value.end(); ++it) {
            std::string key_path = scope.empty() ? it.key() : scope + "." + it.key();

            if (isDangerousKey(it.key())) {
                errors.push_back("Dangerous field '" + key_path + "' is not allowed");
            }

            rejectDangerousKeysRecursive(it.value(), key_path, errors);
        }
    } else if (value.is_array()) {
        for (std::size_t index = 0; index < value.size(); ++index) {
            rejectDangerousKeysRecursive(value.at(index), scope + "[]", errors);
        }
    }
}

void rejectUnknownKeys(
    const nlohmann::json& object,
    const std::vector<std::string>& allowed,
    const std::string& scope,
    std::vector<std::string>& errors
) {
    for (auto it = object.begin(); it != object.end(); ++it) {
        if (!isAllowed(it.key(), allowed)) {
            errors.push_back("Unsupported field '" + scope + it.key() + "'");
        }
    }
}

bool readString(
    const nlohmann::json& input,
    const std::string& key,
    std::string& output,
    std::vector<std::string>& errors,
    bool required
) {
    if (!input.contains(key)) {
        if (required) {
            errors.push_back("Missing required field '" + key + "'");
        }
        return false;
    }

    if (!input.at(key).is_string()) {
        errors.push_back("Field '" + key + "' must be a string");
        return false;
    }

    output = input.at(key).get<std::string>();
    return true;
}

} // namespace

ParsedShapeDefinition parseShapeDefinitionJson(const nlohmann::json& input) {
    ParsedShapeDefinition parsed;

    if (!input.is_object()) {
        parsed.errors.push_back("ShapeDefinition must be a JSON object");
        return parsed;
    }

    rejectDangerousKeysRecursive(input, "", parsed.errors);

    rejectUnknownKeys(
        input,
        {"shape_type", "dimensions", "units", "render", "centered"},
        "",
        parsed.errors
    );

    readString(input, "shape_type", parsed.definition.shape_type, parsed.errors, true);
    readString(input, "units", parsed.definition.units, parsed.errors, false);

    if (input.contains("centered")) {
        if (!input.at("centered").is_boolean()) {
            parsed.errors.push_back("Field 'centered' must be a boolean");
        } else if (!input.at("centered").get<bool>()) {
            parsed.errors.push_back("Field 'centered' must be true");
        }
    }

    if (!input.contains("dimensions")) {
        parsed.errors.push_back("Missing required field 'dimensions'");
    } else if (!input.at("dimensions").is_object()) {
        parsed.errors.push_back("Field 'dimensions' must be an object");
    } else {
        for (auto it = input.at("dimensions").begin(); it != input.at("dimensions").end(); ++it) {
            if (!it.value().is_number()) {
                parsed.errors.push_back("Dimension '" + it.key() + "' must be numeric");
                continue;
            }

            parsed.definition.dimensions[it.key()] = it.value().get<double>();
        }
    }

    if (input.contains("render")) {
        if (!input.at("render").is_object()) {
            parsed.errors.push_back("Field 'render' must be an object");
        } else {
            const auto& render = input.at("render");
            rejectUnknownKeys(render, {"segments", "rings"}, "render.", parsed.errors);

            if (render.contains("segments")) {
                if (!render.at("segments").is_number_integer()) {
                    parsed.errors.push_back("render.segments must be an integer");
                } else {
                    parsed.definition.render.segments = render.at("segments").get<int>();
                }
            }

            if (render.contains("rings")) {
                if (!render.at("rings").is_number_integer()) {
                    parsed.errors.push_back("render.rings must be an integer");
                } else {
                    parsed.definition.render.rings = render.at("rings").get<int>();
                }
            }
        }
    }

    parsed.definition.centered = true;
    return parsed;
}

nlohmann::json shapeDefinitionToJson(const geometry::ShapeDefinition& definition) {
    nlohmann::json render = nlohmann::json::object();

    if (definition.render.segments != 0) {
        render["segments"] = definition.render.segments;
    }

    if (definition.render.rings != 0) {
        render["rings"] = definition.render.rings;
    }

    return {
        {"shape_type", definition.shape_type},
        {"dimensions", definition.dimensions},
        {"units", definition.units},
        {"render", render},
        {"centered", definition.centered}
    };
}

nlohmann::json validationToJson(const geometry::ShapeValidationResult& validation) {
    return {
        {"valid", validation.valid},
        {"errors", validation.errors},
        {"normalized_definition", shapeDefinitionToJson(validation.normalized_definition)}
    };
}

nlohmann::json meshToJson(const core::Mesh& mesh) {
    nlohmann::json vertices = nlohmann::json::array();
    nlohmann::json normals = nlohmann::json::array();
    nlohmann::json edges = nlohmann::json::array();
    nlohmann::json triangles = nlohmann::json::array();

    for (const auto& vertex : mesh.vertices) {
        vertices.push_back({{"x", vertex.x}, {"y", vertex.y}, {"z", vertex.z}});
    }

    for (const auto& normal : mesh.normals) {
        normals.push_back({{"x", normal.x}, {"y", normal.y}, {"z", normal.z}});
    }

    for (const auto& edge : mesh.edges) {
        edges.push_back({{"a", edge.a}, {"b", edge.b}});
    }

    for (const auto& triangle : mesh.triangles) {
        triangles.push_back({{"a", triangle.a}, {"b", triangle.b}, {"c", triangle.c}});
    }

    return {
        {"vertices", vertices},
        {"normals", normals},
        {"edges", edges},
        {"triangles", triangles}
    };
}

nlohmann::json supportedShapesToJson() {
    nlohmann::json shapes = nlohmann::json::array();

    for (const auto& spec : geometry::supportedShapes()) {
        nlohmann::json dimensions = nlohmann::json::object();
        nlohmann::json render = nlohmann::json::object();

        for (const auto& dimension : spec.required_dimensions) {
            dimensions[dimension] = {{"required", true}, {"min_exclusive", 0.0}};
        }

        if (spec.supports_segments) {
            render["segments"] = {
                {"required", false},
                {"min", 8},
                {"max", 256},
                {"default", spec.default_segments}
            };
        }

        if (spec.supports_rings) {
            render["rings"] = {
                {"required", false},
                {"min", 4},
                {"max", 128},
                {"default", spec.default_rings}
            };
        }

        shapes.push_back({
            {"shape_type", spec.shape_type},
            {"dimensions", dimensions},
            {"render", render}
        });
    }

    return shapes;
}

nlohmann::json shapeDefinitionSchemaJson() {
    return {
        {"type", "object"},
        {"additionalProperties", false},
        {"required", {"shape_type", "dimensions", "units", "render", "centered"}},
        {"properties", {
            {"shape_type", {
                {"type", "string"},
                {"enum", {"cube", "cylinder", "pyramid", "sphere"}}
            }},
            {"dimensions", {
                {"type", "object"},
                {"additionalProperties", false},
                {"properties", {
                    {"size", {{"type", "number"}}},
                    {"radius", {{"type", "number"}, {"description", "Radius/radio of a cylinder or sphere."}}},
                    {"height", {{"type", "number"}, {"description", "Height/altura of a cylinder or pyramid."}}},
                    {"base_size", {{"type", "number"}, {"description", "Square base size/tamano de base for a pyramid."}}}
                }}
            }},
            {"units", {{"type", "string"}}},
            {"render", {
                {"type", "object"},
                {"additionalProperties", false},
                {"properties", {
                    {"segments", {
                        {"type", "integer"},
                        {"description", "Number of render segments. Copy explicit values named segments/segmentos from the user prompt."}
                    }},
                    {"rings", {
                        {"type", "integer"},
                        {"description", "Number of sphere rings. Copy explicit values named rings/anillos from the user prompt."}
                    }}
                }}
            }},
            {"centered", {{"type", "boolean"}}}
        }}
    };
}

} // namespace langcad::api
