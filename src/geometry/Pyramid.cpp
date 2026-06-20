#include "langcad/geometry/Pyramid.hpp"

namespace langcad::geometry {

Pyramid::Pyramid(double base_size, double height, core::Vec3 center)
    : base_size_(base_size),
      height_(height),
      center_(center)
{
}

std::string Pyramid::name() const {
    return "Pyramid";
}

core::Mesh Pyramid::toMesh() const {
    core::Mesh mesh;
    double h = base_size_ * 0.5;
    double base_y = -height_ * 0.5;
    double top_y = height_ * 0.5;

    mesh.vertices = {
        center_ + core::Vec3(-h, base_y, -h),
        center_ + core::Vec3( h, base_y, -h),
        center_ + core::Vec3( h, base_y,  h),
        center_ + core::Vec3(-h, base_y,  h),
        center_ + core::Vec3(0.0, top_y, 0.0)
    };

    mesh.edges = {
        {0, 1}, {1, 2}, {2, 3}, {3, 0},
        {0, 4}, {1, 4}, {2, 4}, {3, 4}
    };

    return mesh;
}

} // namespace langcad::geometry
