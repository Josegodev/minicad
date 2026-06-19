#include "Mesh.hpp"

namespace minicad::geometry {

Mesh Mesh::cube(double size) {
    Mesh mesh;
    double h = size * 0.5;

    mesh.vertices_ = {
        {-h, -h, -h},
        { h, -h, -h},
        { h,  h, -h},
        {-h,  h, -h},
        {-h, -h,  h},
        { h, -h,  h},
        { h,  h,  h},
        {-h,  h,  h}
    };

    mesh.edges_ = {
        {{0, 1}}, {{1, 2}}, {{2, 3}}, {{3, 0}},
        {{4, 5}}, {{5, 6}}, {{6, 7}}, {{7, 4}},
        {{0, 4}}, {{1, 5}}, {{2, 6}}, {{3, 7}}
    };

    return mesh;
}

const std::vector<minicad::core::Vec3>& Mesh::vertices() const {
    return vertices_;
}

const std::vector<std::array<int, 2>>& Mesh::edges() const {
    return edges_;
}

} // namespace minicad::geometry
