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

    auto addTriangle = [&mesh, this](const core::Vec3& a, const core::Vec3& b, const core::Vec3& c) {
        core::Vec3 normal = (b - a).cross(c - a).normalized();
        int start = static_cast<int>(mesh.vertices.size());

        mesh.vertices.push_back(center_ + a);
        mesh.vertices.push_back(center_ + b);
        mesh.vertices.push_back(center_ + c);
        mesh.normals.push_back(normal);
        mesh.normals.push_back(normal);
        mesh.normals.push_back(normal);
        mesh.triangles.push_back({start, start + 1, start + 2});
        mesh.edges.push_back({start, start + 1});
        mesh.edges.push_back({start + 1, start + 2});
        mesh.edges.push_back({start + 2, start});
    };

    core::Vec3 v0(-h, base_y, -h);
    core::Vec3 v1( h, base_y, -h);
    core::Vec3 v2( h, base_y,  h);
    core::Vec3 v3(-h, base_y,  h);
    core::Vec3 top(0.0, top_y, 0.0);

    addTriangle(v0, v2, v1);
    addTriangle(v0, v3, v2);
    addTriangle(v0, v1, top);
    addTriangle(v1, v2, top);
    addTriangle(v2, v3, top);
    addTriangle(v3, v0, top);

    return mesh;
}

#if 0

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
#endif

} // namespace langcad::geometry
