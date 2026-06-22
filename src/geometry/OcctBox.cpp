#include "langcad/geometry/OcctBox.hpp"

#include "langcad/geometry/OcctMeshConverter.hpp"

#include <BRepPrimAPI_MakeBox.hxx>
#include <gp_Pnt.hxx>

namespace langcad::geometry {

OcctBox::OcctBox(double size, core::Vec3 center)
    : size_(size),
      center_(center)
{
}

std::string OcctBox::name() const {
    return "OCC Box";
}

core::Mesh OcctBox::toMesh() const {
    const double h = size_ * 0.5;
    const gp_Pnt min_point(center_.x - h, center_.y - h, center_.z - h);
    const gp_Pnt max_point(center_.x + h, center_.y + h, center_.z + h);
    return meshFromOcctShape(BRepPrimAPI_MakeBox(min_point, max_point).Shape());
}

} // namespace langcad::geometry
