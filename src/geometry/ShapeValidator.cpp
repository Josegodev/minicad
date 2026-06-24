#include "langcad/geometry/ShapeValidator.hpp"

#include "langcad/geometry/ShapeCatalog.hpp"

#include <algorithm>
#include <cmath>
#include <set>

namespace langcad::geometry {

namespace {

constexpr int min_segments = 8;
constexpr int max_segments = 256;
constexpr int min_rings = 4;
constexpr int max_rings = 128;

bool contains(const std::vector<std::string>& values, const std::string& value) {
    return std::find(values.begin(), values.end(), value) != values.end();
}

bool isFacetedShape(const ShapeDefinition& definition) {
    return definition.shape_type == "faceted_shape";
}

bool hasExplicitTopology(const ShapeDefinition& definition) {
    return !definition.vertices.empty() || !definition.edges.empty() || !definition.faces.empty();
}

bool isFinite(const core::Vec3& vertex) {
    return std::isfinite(vertex.x) && std::isfinite(vertex.y) && std::isfinite(vertex.z);
}

bool validIndex(int index, std::size_t size) {
    return index >= 0 && static_cast<std::size_t>(index) < size;
}

std::vector<core::Edge> deriveEdgesFromFaces(const std::vector<FaceDefinition>& faces) {
    std::vector<core::Edge> edges;
    std::set<std::pair<int, int>> seen_edges;

    for (const auto& face : faces) {
        for (std::size_t index = 0; index < face.vertices.size(); ++index) {
            int a = face.vertices[index];
            int b = face.vertices[(index + 1) % face.vertices.size()];
            auto edge_key = std::minmax(a, b);

            if (seen_edges.insert(edge_key).second) {
                edges.push_back({a, b});
            }
        }
    }

    return edges;
}

void addUnsupportedDimensionErrors(
    const ShapeDefinition& definition,
    const ShapeSpec& spec,
    ShapeValidationResult& result
) {
    for (const auto& [name, value] : definition.dimensions) {
        (void)value;

        if (!contains(spec.required_dimensions, name)) {
            result.errors.push_back("Unsupported dimension '" + name + "' for shape_type '" + spec.shape_type + "'");
        }
    }
}

void validateFacetedShape(const ShapeDefinition& definition, ShapeValidationResult& result) {
    if (definition.vertices.empty()) {
        result.errors.push_back("faceted_shape requires at least one vertex");
        return;
    }

    if (definition.edges.empty() && definition.faces.empty()) {
        result.errors.push_back("faceted_shape requires faces for a solid or edges for a wire shape");
    }

    for (std::size_t index = 0; index < definition.vertices.size(); ++index) {
        if (!isFinite(definition.vertices[index])) {
            result.errors.push_back("vertices[" + std::to_string(index) + "] must contain finite coordinates");
        }
    }

    std::set<std::pair<int, int>> seen_edges;
    for (std::size_t index = 0; index < definition.edges.size(); ++index) {
        const auto& edge = definition.edges[index];

        if (!validIndex(edge.a, definition.vertices.size()) || !validIndex(edge.b, definition.vertices.size())) {
            result.errors.push_back("edges[" + std::to_string(index) + "] references a vertex index outside vertices");
            continue;
        }

        if (edge.a == edge.b) {
            result.errors.push_back("edges[" + std::to_string(index) + "] must reference two different vertices");
            continue;
        }

        if (!seen_edges.insert(std::minmax(edge.a, edge.b)).second) {
            result.errors.push_back("edges[" + std::to_string(index) + "] duplicates another edge");
        }
    }

    for (std::size_t face_index = 0; face_index < definition.faces.size(); ++face_index) {
        const auto& face = definition.faces[face_index];

        if (face.vertices.size() < 3) {
            result.errors.push_back("faces[" + std::to_string(face_index) + "] requires at least three vertex indices");
            continue;
        }

        std::set<int> seen_face_vertices;
        bool face_indices_valid = true;
        for (int vertex_index : face.vertices) {
            if (!validIndex(vertex_index, definition.vertices.size())) {
                result.errors.push_back("faces[" + std::to_string(face_index) + "] references a vertex index outside vertices");
                face_indices_valid = false;
                continue;
            }

            if (!seen_face_vertices.insert(vertex_index).second) {
                result.errors.push_back("faces[" + std::to_string(face_index) + "] repeats vertex index " + std::to_string(vertex_index));
                face_indices_valid = false;
            }
        }

        if (face_indices_valid && seen_face_vertices.size() >= 3) {
            const core::Vec3& anchor = definition.vertices[face.vertices.front()];
            bool has_area = false;

            for (std::size_t index = 1; index + 1 < face.vertices.size(); ++index) {
                core::Vec3 normal = (definition.vertices[face.vertices[index]] - anchor)
                    .cross(definition.vertices[face.vertices[index + 1]] - anchor);

                if (normal.norm() > 0.0) {
                    has_area = true;
                    break;
                }
            }

            if (!has_area) {
                result.errors.push_back("faces[" + std::to_string(face_index) + "] is degenerate");
            }
        }
    }

    if (!definition.faces.empty()) {
        result.normalized_definition.edges = deriveEdgesFromFaces(definition.faces);
    }
}

} // namespace

ShapeValidationResult validateShapeDefinition(const ShapeDefinition& definition) {
    ShapeValidationResult result;
    result.normalized_definition = definition;
    result.normalized_definition.centered = true;

    const ShapeSpec* spec = findShapeSpec(definition.shape_type);
    if (!spec) {
        result.errors.push_back("Unsupported shape_type '" + definition.shape_type + "'");
        return result;
    }

    if (!isFacetedShape(definition) && hasExplicitTopology(definition)) {
        result.errors.push_back("vertices, edges and faces are only supported for shape_type 'faceted_shape'");
    }

    addUnsupportedDimensionErrors(definition, *spec, result);

    if (isFacetedShape(definition)) {
        validateFacetedShape(definition, result);
    }

    for (const auto& dimension : spec->required_dimensions) {
        auto found = definition.dimensions.find(dimension);

        if (found == definition.dimensions.end()) {
            result.errors.push_back("Missing required dimension '" + dimension + "'");
            continue;
        }

        if (found->second <= 0.0) {
            result.errors.push_back("Dimension '" + dimension + "' must be positive");
        }
    }

    if (!spec->supports_segments && definition.tessellation.segments != 0) {
        result.errors.push_back("render.segments is not supported for shape_type '" + spec->shape_type + "'");
    }

    if (!spec->supports_rings && definition.tessellation.rings != 0) {
        result.errors.push_back("render.rings is not supported for shape_type '" + spec->shape_type + "'");
    }

    if (spec->supports_segments) {
        int segments = definition.tessellation.segments == 0
            ? spec->default_segments
            : definition.tessellation.segments;

        if (segments < min_segments || segments > max_segments) {
            result.errors.push_back("render.segments must be between 8 and 256");
        } else {
            result.normalized_definition.tessellation.segments = segments;
        }
    }

    if (spec->supports_rings) {
        int rings = definition.tessellation.rings == 0
            ? spec->default_rings
            : definition.tessellation.rings;

        if (rings < min_rings || rings > max_rings) {
            result.errors.push_back("render.rings must be between 4 and 128");
        } else {
            result.normalized_definition.tessellation.rings = rings;
        }
    }

    result.valid = result.errors.empty();
    return result;
}

} // namespace langcad::geometry
