#include "langcad/geometry/Cylinder.hpp"

#include <algorithm>
#include <cmath>

namespace langcad::geometry {

namespace {

constexpr double pi = 3.14159265358979323846;

} // namespace

Cylinder::Cylinder(double radius, double height, int segments, core::Vec3 center)
    : radius_(radius),
      height_(height),
      segments_(std::max(segments, 3)),
      center_(center)
{
}

std::string Cylinder::name() const {
    return "Cylinder";
}

core::Mesh Cylinder::toMesh() const {
    core::Mesh mesh;
    double half_height = height_ * 0.5;

    mesh.vertices.reserve(static_cast<std::size_t>(segments_) * 2);
    mesh.edges.reserve(static_cast<std::size_t>(segments_) * 3);

    for (int i = 0; i < segments_; ++i) {
        double angle = (2.0 * pi * static_cast<double>(i)) / static_cast<double>(segments_);
        double x = radius_ * std::cos(angle);
        double z = radius_ * std::sin(angle);

        mesh.vertices.push_back(center_ + core::Vec3(x, -half_height, z));
    }

    for (int i = 0; i < segments_; ++i) {
        double angle = (2.0 * pi * static_cast<double>(i)) / static_cast<double>(segments_);
        double x = radius_ * std::cos(angle);
        double z = radius_ * std::sin(angle);

        mesh.vertices.push_back(center_ + core::Vec3(x, half_height, z));
    }

    for (int i = 0; i < segments_; ++i) {
        int next = (i + 1) % segments_;
        int top_current = i + segments_;
        int top_next = next + segments_;

        mesh.edges.push_back({i, next});
        mesh.edges.push_back({top_current, top_next});
        mesh.edges.push_back({i, top_current});
    }

    return mesh;
}

} // namespace langcad::geometry
