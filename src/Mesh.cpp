#include "Mesh.hpp"

#include <algorithm>
#include <cmath>

namespace minicad::geometry {

namespace {

constexpr double pi = 3.14159265358979323846;

} // namespace

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

Mesh Mesh::cylinder(double radius, double height, int segments) {
    Mesh mesh;
    int clamped_segments = std::max(segments, 3);
    double half_height = height * 0.5;

    mesh.vertices_.reserve(static_cast<std::size_t>(clamped_segments) * 2);
    mesh.edges_.reserve(static_cast<std::size_t>(clamped_segments) * 3);

    for (int i = 0; i < clamped_segments; ++i) {
        double angle = (2.0 * pi * static_cast<double>(i)) / static_cast<double>(clamped_segments);
        double x = radius * std::cos(angle);
        double z = radius * std::sin(angle);

        mesh.vertices_.push_back({x, -half_height, z});
    }

    for (int i = 0; i < clamped_segments; ++i) {
        double angle = (2.0 * pi * static_cast<double>(i)) / static_cast<double>(clamped_segments);
        double x = radius * std::cos(angle);
        double z = radius * std::sin(angle);

        mesh.vertices_.push_back({x, half_height, z});
    }

    for (int i = 0; i < clamped_segments; ++i) {
        int next = (i + 1) % clamped_segments;
        int top_current = i + clamped_segments;
        int top_next = next + clamped_segments;

        mesh.edges_.push_back({{i, next}});
        mesh.edges_.push_back({{top_current, top_next}});
        mesh.edges_.push_back({{i, top_current}});
    }

    return mesh;
}

const std::vector<minicad::core::Vec3>& Mesh::vertices() const {
    return vertices_;
}

const std::vector<std::array<int, 2>>& Mesh::edges() const {
    return edges_;
}

} // namespace minicad::geometry
