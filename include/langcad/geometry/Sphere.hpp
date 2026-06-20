#pragma once

#include "langcad/core/Vec3.hpp"
#include "langcad/geometry/Shape3D.hpp"

namespace langcad::geometry {

class Sphere final : public Shape3D {
public:
    Sphere(double radius = 0.5, int rings = 12, int segments = 24, core::Vec3 center = core::Vec3());

    std::string name() const override;
    core::Mesh toMesh() const override;

private:
    double radius_;
    int rings_;
    int segments_;
    core::Vec3 center_;
};

} // namespace langcad::geometry
