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

    auto addFace = [&mesh, this](
        const core::Vec3& a,
        const core::Vec3& b,
        const core::Vec3& c,
        const core::Vec3& d,
        const core::Vec3& normal
    ) {
        int start = static_cast<int>(mesh.vertices.size());
        mesh.vertices.push_back(center_ + a);
        mesh.vertices.push_back(center_ + b);
        mesh.vertices.push_back(center_ + c);
        mesh.vertices.push_back(center_ + d);

        mesh.normals.push_back(normal);
        mesh.normals.push_back(normal);
        mesh.normals.push_back(normal);
        mesh.normals.push_back(normal);

        mesh.triangles.push_back({start, start + 1, start + 2});
        mesh.triangles.push_back({start, start + 2, start + 3});

        mesh.edges.push_back({start, start + 1});
        mesh.edges.push_back({start + 1, start + 2});
        mesh.edges.push_back({start + 2, start + 3});
        mesh.edges.push_back({start + 3, start});
    };

    addFace(core::Vec3(-h, -h,  h), core::Vec3( h, -h,  h), core::Vec3( h,  h,  h), core::Vec3(-h,  h,  h), core::Vec3(0.0, 0.0, 1.0));
    addFace(core::Vec3( h, -h, -h), core::Vec3(-h, -h, -h), core::Vec3(-h,  h, -h), core::Vec3( h,  h, -h), core::Vec3(0.0, 0.0, -1.0));
    addFace(core::Vec3(-h, -h, -h), core::Vec3(-h, -h,  h), core::Vec3(-h,  h,  h), core::Vec3(-h,  h, -h), core::Vec3(-1.0, 0.0, 0.0));
    addFace(core::Vec3( h, -h,  h), core::Vec3( h, -h, -h), core::Vec3( h,  h, -h), core::Vec3( h,  h,  h), core::Vec3(1.0, 0.0, 0.0));
    addFace(core::Vec3(-h,  h,  h), core::Vec3( h,  h,  h), core::Vec3( h,  h, -h), core::Vec3(-h,  h, -h), core::Vec3(0.0, 1.0, 0.0));
    addFace(core::Vec3(-h, -h, -h), core::Vec3( h, -h, -h), core::Vec3( h, -h,  h), core::Vec3(-h, -h,  h), core::Vec3(0.0, -1.0, 0.0));

    return mesh;
}

#if 0
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
#endif

} // namespace langcad::geometry
