#pragma once

#include <string>
#include <vector>

namespace langcad::geometry {

struct ShapeSpec {
    std::string shape_type;
    std::vector<std::string> required_dimensions;
    bool supports_segments;
    bool supports_rings;
    int default_segments;
    int default_rings;
};

const std::vector<ShapeSpec>& supportedShapes();
const ShapeSpec* findShapeSpec(const std::string& shape_type);

} // namespace langcad::geometry
