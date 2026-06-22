#include "langcad/geometry/OcctSphere.hpp"

#include "langcad/geometry/OcctMeshConverter.hpp"

#include <BRepPrimAPI_MakeSphere.hxx>
#include <gp_Pnt.hxx>

namespace langcad::geometry {

OcctSphere::OcctSphere(double radius, core::Vec3 center)
    : radius_(radius),
      center_(center)
{
}

std::string OcctSphere::name() const {
    return "OCC Sphere";
}

core::Mesh OcctSphere::toMesh() const {
    return meshFromOcctShape(BRepPrimAPI_MakeSphere(gp_Pnt(center_.x, center_.y, center_.z), radius_).Shape());
}

} // namespace langcad::geometry
