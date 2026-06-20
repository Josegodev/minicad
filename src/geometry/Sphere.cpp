#include "langcad/geometry/Sphere.hpp"

#include <algorithm>
#include <cmath>

namespace langcad::geometry {

namespace {

constexpr double pi = 3.14159265358979323846;

} // namespace

Sphere::Sphere(double radius, int rings, int segments, core::Vec3 center)
    : radius_(radius),
      rings_(std::max(rings, 3)),
      segments_(std::max(segments, 3)),
      center_(center)
{
}

std::string Sphere::name() const {
    return "Sphere";
}

core::Mesh Sphere::toMesh() const {
    core::Mesh mesh;

    // The sphere is represented as deterministic latitude and longitude wire rings.
    mesh.vertices.reserve(static_cast<std::size_t>(rings_ + 1) * static_cast<std::size_t>(segments_));
    mesh.edges.reserve(static_cast<std::size_t>(rings_ + 1) * static_cast<std::size_t>(segments_) * 2);

    for (int ring = 0; ring <= rings_; ++ring) {
        double v = static_cast<double>(ring) / static_cast<double>(rings_);
        double latitude = -pi * 0.5 + pi * v;
        double y = radius_ * std::sin(latitude);
        double ring_radius = radius_ * std::cos(latitude);

        for (int segment = 0; segment < segments_; ++segment) {
            double u = static_cast<double>(segment) / static_cast<double>(segments_);
            double angle = 2.0 * pi * u;

            mesh.vertices.push_back(center_ + core::Vec3(
                ring_radius * std::cos(angle),
                y,
                ring_radius * std::sin(angle)
            ));
        }
    }

    for (int ring = 0; ring <= rings_; ++ring) {
        for (int segment = 0; segment < segments_; ++segment) {
            int current = ring * segments_ + segment;
            int next_segment = ring * segments_ + ((segment + 1) % segments_);

            mesh.edges.push_back({current, next_segment});

            if (ring < rings_) {
                int next_ring = (ring + 1) * segments_ + segment;
                mesh.edges.push_back({current, next_ring});
            }
        }
    }

    return mesh;
}

} // namespace langcad::geometry
