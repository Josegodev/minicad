#include "langcad/geometry/ShapeCatalog.hpp"

namespace langcad::geometry {

const std::vector<ShapeSpec>& supportedShapes() {
    static const std::vector<ShapeSpec> shapes = {
        {"cube", {"size"}, false, false, 0, 0},
        {"cylinder", {"radius", "height"}, true, false, 32, 0},
        {"pyramid", {"base_size", "height"}, false, false, 0, 0},
        {"sphere", {"radius"}, true, true, 24, 12},
        {"faceted_shape", {}, false, false, 0, 0}
    };

    return shapes;
}

const ShapeSpec* findShapeSpec(const std::string& shape_type) {
    for (const auto& spec : supportedShapes()) {
        if (spec.shape_type == shape_type) {
            return &spec;
        }
    }

    return nullptr;
}

} // namespace langcad::geometry
