#include "langcad/geometry/Cube.hpp"

namespace langcad::geometry {

Cube::Cube(double size, core::Vec3 center)
    : size_(size),
      center_(center)
{
}

std::string Cube::name() const {
    return "Cube";
}

core::Mesh Cube::toMesh() const {
    core::Mesh mesh;
    double h = size_ * 0.5;

    mesh.vertices = {
        center_ + core::Vec3(-h, -h, -h),
        center_ + core::Vec3( h, -h, -h),
        center_ + core::Vec3( h,  h, -h),
        center_ + core::Vec3(-h,  h, -h),
        center_ + core::Vec3(-h, -h,  h),
        center_ + core::Vec3( h, -h,  h),
        center_ + core::Vec3( h,  h,  h),
        center_ + core::Vec3(-h,  h,  h)
    };

    mesh.edges = {
        {0, 1}, {1, 2}, {2, 3}, {3, 0},
        {4, 5}, {5, 6}, {6, 7}, {7, 4},
        {0, 4}, {1, 5}, {2, 6}, {3, 7}
    };

    return mesh;
}

} // namespace langcad::geometry
