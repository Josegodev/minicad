#pragma once

#include "langcad/core/Vec3.hpp"
#include "langcad/geometry/Shape3D.hpp"

namespace langcad::geometry {

class Cylinder final : public Shape3D {
public:
    Cylinder(double radius = 0.5, double height = 1.0, int segments = 32, core::Vec3 center = core::Vec3());

    std::string name() const override;
    core::Mesh toMesh() const override;

private:
    double radius_;
    double height_;
    int segments_;
    core::Vec3 center_;
};

} // namespace langcad::geometry
