#include "langcad/geometry/ShapeFactory.hpp"

#include "langcad/geometry/Cube.hpp"
#include "langcad/geometry/Cylinder.hpp"
#include "langcad/geometry/FacetedShape.hpp"
#include "langcad/geometry/OcctBox.hpp"
#include "langcad/geometry/OcctCylinder.hpp"
#include "langcad/geometry/OcctSphere.hpp"
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
    case 5:
        return std::make_unique<OcctBox>();
    case 6:
        return std::make_unique<OcctCylinder>();
    case 7:
        return std::make_unique<OcctSphere>();
    default:
        throw std::invalid_argument("Invalid shape selection");
    }
}

std::unique_ptr<Shape3D> createShapeFromDefinition(const ShapeDefinition& definition) {
    const core::Vec3 origin;

    if (definition.shape_type == "cube") {
        return std::make_unique<Cube>(
            definition.dimensions.at("size"),
            origin
        );
    }

    if (definition.shape_type == "cylinder") {
        return std::make_unique<Cylinder>(
            definition.dimensions.at("radius"),
            definition.dimensions.at("height"),
            definition.tessellation.segments,
            origin
        );
    }

    if (definition.shape_type == "pyramid") {
        return std::make_unique<Pyramid>(
            definition.dimensions.at("base_size"),
            definition.dimensions.at("height"),
            origin
        );
    }

    if (definition.shape_type == "sphere") {
        return std::make_unique<Sphere>(
            definition.dimensions.at("radius"),
            definition.tessellation.rings,
            definition.tessellation.segments,
            origin
        );
    }

    if (definition.shape_type == "faceted_shape") {
        return std::make_unique<FacetedShape>(
            definition.vertices,
            definition.edges,
            definition.faces
        );
    }

    throw std::invalid_argument("Unsupported shape definition");
}

} // namespace langcad::geometry
