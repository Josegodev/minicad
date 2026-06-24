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

bool readInteger(
    const nlohmann::json& input,
    const std::string& key,
    int& output,
    const std::string& scope,
    std::vector<std::string>& errors
) {
    if (!input.contains(key)) {
        errors.push_back("Missing required field '" + scope + key + "'");
        return false;
    }

    if (!input.at(key).is_number_integer()) {
        errors.push_back("Field '" + scope + key + "' must be an integer");
        return false;
    }

    output = input.at(key).get<int>();
    return true;
}

bool readNumber(
    const nlohmann::json& input,
    const std::string& key,
    double& output,
    const std::string& scope,
    std::vector<std::string>& errors
) {
    if (!input.contains(key)) {
        errors.push_back("Missing required field '" + scope + key + "'");
        return false;
    }

    if (!input.at(key).is_number()) {
        errors.push_back("Field '" + scope + key + "' must be numeric");
        return false;
    }

    output = input.at(key).get<double>();
    return true;
}

void parseVertices(const nlohmann::json& input, geometry::ShapeDefinition& definition, std::vector<std::string>& errors) {
    if (!input.contains("vertices")) {
        return;
    }

    if (!input.at("vertices").is_array()) {
        errors.push_back("Field 'vertices' must be an array");
        return;
    }

    for (std::size_t index = 0; index < input.at("vertices").size(); ++index) {
        const auto& vertex = input.at("vertices").at(index);
        std::string scope = "vertices[" + std::to_string(index) + "].";

        if (!vertex.is_object()) {
            errors.push_back("vertices[" + std::to_string(index) + "] must be an object");
            continue;
        }

        rejectUnknownKeys(vertex, {"x", "y", "z"}, scope, errors);

        double x = 0.0;
        double y = 0.0;
        double z = 0.0;
        bool ok = readNumber(vertex, "x", x, scope, errors);
        ok = readNumber(vertex, "y", y, scope, errors) && ok;
        ok = readNumber(vertex, "z", z, scope, errors) && ok;

        if (ok) {
            definition.vertices.push_back(core::Vec3(x, y, z));
        }
    }
}

void parseEdges(const nlohmann::json& input, geometry::ShapeDefinition& definition, std::vector<std::string>& errors) {
    if (!input.contains("edges")) {
        return;
    }

    if (!input.at("edges").is_array()) {
        errors.push_back("Field 'edges' must be an array");
        return;
    }

    for (std::size_t index = 0; index < input.at("edges").size(); ++index) {
        const auto& edge = input.at("edges").at(index);
        std::string scope = "edges[" + std::to_string(index) + "].";

        if (!edge.is_object()) {
            errors.push_back("edges[" + std::to_string(index) + "] must be an object");
            continue;
        }

        rejectUnknownKeys(edge, {"a", "b"}, scope, errors);

        int a = 0;
        int b = 0;
        bool ok = readInteger(edge, "a", a, scope, errors);
        ok = readInteger(edge, "b", b, scope, errors) && ok;

        if (ok) {
            definition.edges.push_back({a, b});
        }
    }
}

void parseFaces(const nlohmann::json& input, geometry::ShapeDefinition& definition, std::vector<std::string>& errors) {
    if (!input.contains("faces")) {
        return;
    }

    if (!input.at("faces").is_array()) {
        errors.push_back("Field 'faces' must be an array");
        return;
    }

    for (std::size_t face_index = 0; face_index < input.at("faces").size(); ++face_index) {
        const auto& face_json = input.at("faces").at(face_index);

        if (!face_json.is_array()) {
            errors.push_back("faces[" + std::to_string(face_index) + "] must be an array of vertex indices");
            continue;
        }

        geometry::FaceDefinition face;
        for (std::size_t vertex_index = 0; vertex_index < face_json.size(); ++vertex_index) {
            if (!face_json.at(vertex_index).is_number_integer()) {
                errors.push_back("faces[" + std::to_string(face_index) + "][" + std::to_string(vertex_index) + "] must be an integer");
                continue;
            }

            face.vertices.push_back(face_json.at(vertex_index).get<int>());
        }

        definition.faces.push_back(face);
    }
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
        {"shape_type", "dimensions", "vertices", "edges", "faces", "units", "render", "centered"},
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
                    parsed.definition.tessellation.segments = render.at("segments").get<int>();
                }
            }

            if (render.contains("rings")) {
                if (!render.at("rings").is_number_integer()) {
                    parsed.errors.push_back("render.rings must be an integer");
                } else {
                    parsed.definition.tessellation.rings = render.at("rings").get<int>();
                }
            }
        }
    }

    parseVertices(input, parsed.definition, parsed.errors);
    parseEdges(input, parsed.definition, parsed.errors);
    parseFaces(input, parsed.definition, parsed.errors);

    if (!parsed.definition.shape_type.empty()
        && parsed.definition.shape_type != "faceted_shape"
        && (!parsed.definition.vertices.empty() || !parsed.definition.edges.empty() || !parsed.definition.faces.empty())) {
        parsed.errors.push_back("vertices, edges and faces are only supported for shape_type 'faceted_shape'");
    }

    parsed.definition.centered = true;
    return parsed;
}

nlohmann::json shapeDefinitionToJson(const geometry::ShapeDefinition& definition) {
    nlohmann::json render = nlohmann::json::object();
    nlohmann::json output = {
        {"shape_type", definition.shape_type},
        {"dimensions", definition.dimensions},
        {"units", definition.units},
        {"render", render},
        {"centered", definition.centered}
    };

    if (definition.tessellation.segments != 0) {
        output["render"]["segments"] = definition.tessellation.segments;
    }

    if (definition.tessellation.rings != 0) {
        output["render"]["rings"] = definition.tessellation.rings;
    }

    if (definition.shape_type == "faceted_shape" || !definition.vertices.empty() || !definition.edges.empty() || !definition.faces.empty()) {
        nlohmann::json vertices = nlohmann::json::array();
        nlohmann::json edges = nlohmann::json::array();
        nlohmann::json faces = nlohmann::json::array();

        for (const auto& vertex : definition.vertices) {
            vertices.push_back({{"x", vertex.x}, {"y", vertex.y}, {"z", vertex.z}});
        }

        for (const auto& edge : definition.edges) {
            edges.push_back({{"a", edge.a}, {"b", edge.b}});
        }

        for (const auto& face : definition.faces) {
            faces.push_back(face.vertices);
        }

        output["vertices"] = vertices;
        output["edges"] = edges;
        output["faces"] = faces;
    }

    return output;
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

        nlohmann::json shape = {
            {"shape_type", spec.shape_type},
            {"dimensions", dimensions},
            {"render", render}
        };

        if (spec.shape_type == "faceted_shape") {
            shape["vertices"] = {
                {"required", true},
                {"item", {{"x", "number"}, {"y", "number"}, {"z", "number"}}}
            };
            shape["edges"] = {
                {"required", false},
                {"item", {{"a", "integer vertex index"}, {"b", "integer vertex index"}}}
            };
            shape["faces"] = {
                {"required", false},
                {"item", "array of at least three integer vertex indices"}
            };
        }

        shapes.push_back(shape);
    }

    return shapes;
}

nlohmann::json shapeDefinitionSchemaJson() {
    return {
        {"type", "object"},
        {"additionalProperties", false},
        {"required", {"shape_type", "dimensions", "vertices", "faces", "units", "render", "centered"}},
        {"properties", {
            {"shape_type", {
                {"type", "string"},
                {"enum", {"faceted_shape"}}
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
            {"vertices", {
                {"type", "array"},
                {"items", {
                    {"type", "object"},
                    {"additionalProperties", false},
                    {"required", {"x", "y", "z"}},
                    {"properties", {
                        {"x", {{"type", "number"}}},
                        {"y", {{"type", "number"}}},
                        {"z", {{"type", "number"}}}
                    }}
                }}
            }},
            {"faces", {
                {"type", "array"},
                {"minItems", 1},
                {"items", {
                    {"type", "array"},
                    {"minItems", 3},
                    {"items", {{"type", "integer"}}}
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
