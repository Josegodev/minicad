#include "langcad/geometry/ShapeValidator.hpp"

#include "langcad/geometry/ShapeCatalog.hpp"

#include <algorithm>

namespace langcad::geometry {

namespace {

constexpr int min_segments = 8;
constexpr int max_segments = 256;
constexpr int min_rings = 4;
constexpr int max_rings = 128;

bool contains(const std::vector<std::string>& values, const std::string& value) {
    return std::find(values.begin(), values.end(), value) != values.end();
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

    addUnsupportedDimensionErrors(definition, *spec, result);

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

    if (!spec->supports_segments && definition.render.segments != 0) {
        result.errors.push_back("render.segments is not supported for shape_type '" + spec->shape_type + "'");
    }

    if (!spec->supports_rings && definition.render.rings != 0) {
        result.errors.push_back("render.rings is not supported for shape_type '" + spec->shape_type + "'");
    }

    if (spec->supports_segments) {
        int segments = definition.render.segments == 0
            ? spec->default_segments
            : definition.render.segments;

        if (segments < min_segments || segments > max_segments) {
            result.errors.push_back("render.segments must be between 8 and 256");
        } else {
            result.normalized_definition.render.segments = segments;
        }
    }

    if (spec->supports_rings) {
        int rings = definition.render.rings == 0
            ? spec->default_rings
            : definition.render.rings;

        if (rings < min_rings || rings > max_rings) {
            result.errors.push_back("render.rings must be between 4 and 128");
        } else {
            result.normalized_definition.render.rings = rings;
        }
    }

    result.valid = result.errors.empty();
    return result;
}

} // namespace langcad::geometry
