#pragma once

#include "langcad/core/Vec3.hpp"
#include "langcad/geometry/Shape3D.hpp"

namespace langcad::geometry {

class Pyramid final : public Shape3D {
public:
    Pyramid(double base_size = 1.0, double height = 1.0, core::Vec3 center = core::Vec3());

    std::string name() const override;
    core::Mesh toMesh() const override;

private:
    double base_size_;
    double height_;
    core::Vec3 center_;
};

} // namespace langcad::geometry
