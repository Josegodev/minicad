#include "langcad/geometry/ShapeFactory.hpp"

#include "langcad/geometry/Cube.hpp"
#include "langcad/geometry/Cylinder.hpp"
#include "langcad/geometry/Pyramid.hpp"
#include "langcad/geometry/Sphere.hpp"

#include <stdexcept>

namespace langcad::geometry {

std::unique_ptr<Shape3D> createShapeFromSelection(int selection) {
    switch (selection) {
    case 1:
        return std::make_unique<Cube>();
    case 2:
        return std::make_unique<Cylinder>();
    case 3:
        return std::make_unique<Pyramid>();
    case 4:
        return std::make_unique<Sphere>();
    default:
        throw std::invalid_argument("Invalid shape selection");
    }
}

} // namespace langcad::geometry
