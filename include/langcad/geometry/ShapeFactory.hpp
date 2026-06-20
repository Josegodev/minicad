#pragma once

#include "langcad/geometry/Shape3D.hpp"

#include <memory>

namespace langcad::geometry {

std::unique_ptr<Shape3D> createShapeFromSelection(int selection);

} // namespace langcad::geometry
