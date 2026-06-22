#include "langcad/geometry/OcctCylinder.hpp"

#include "langcad/geometry/OcctMeshConverter.hpp"

#include <BRepPrimAPI_MakeCylinder.hxx>
#include <gp_Ax2.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>

namespace langcad::geometry {

OcctCylinder::OcctCylinder(double radius, double height, core::Vec3 center)
    : radius_(radius),
      height_(height),
      center_(center)
{
}

std::string OcctCylinder::name() const {
    return "OCC Cylinder";
}

core::Mesh OcctCylinder::toMesh() const {
    const gp_Pnt base_center(center_.x, center_.y - height_ * 0.5, center_.z);
    const gp_Ax2 axis(base_center, gp_Dir(0.0, 1.0, 0.0));
    return meshFromOcctShape(BRepPrimAPI_MakeCylinder(axis, radius_, height_).Shape());
}

} // namespace langcad::geometry
