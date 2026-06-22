#pragma once

#include "langcad/geometry/Shape3D.hpp"
#include "langcad/geometry/ShapeDefinition.hpp"

#include <memory>

namespace langcad::geometry {

std::unique_ptr<Shape3D> createShapeFromSelection(int selection);
std::unique_ptr<Shape3D> createShapeFromDefinition(const ShapeDefinition& definition);

} // namespace langcad::geometry
