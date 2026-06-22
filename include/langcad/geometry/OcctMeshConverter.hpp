#pragma once

#include "langcad/core/Mesh.hpp"

class TopoDS_Shape;

namespace langcad::geometry {

core::Mesh meshFromOcctShape(const TopoDS_Shape& shape, double linear_deflection = 0.05);

} // namespace langcad::geometry
