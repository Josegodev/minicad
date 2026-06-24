#include "langcad/geometry/FacetedShape.hpp"

#include <algorithm>
#include <utility>
#include <vector>

namespace langcad::geometry {

namespace {

void addNormal(core::Vec3& target, const core::Vec3& value) {
    target.x += value.x;
    target.y += value.y;
    target.z += value.z;
}

void addFaceEdges(core::Mesh& mesh, const FaceDefinition& face) {
    for (std::size_t index = 0; index < face.vertices.size(); ++index) {
        int a = face.vertices[index];
        int b = face.vertices[(index + 1) % face.vertices.size()];
        auto edge = std::minmax(a, b);

        auto found = std::find_if(mesh.edges.begin(), mesh.edges.end(), [edge](const core::Edge& existing) {
            auto existing_edge = std::minmax(existing.a, existing.b);
            return existing_edge.first == edge.first && existing_edge.second == edge.second;
        });

        if (found == mesh.edges.end()) {
            mesh.edges.push_back({a, b});
        }
    }
}

} // namespace

FacetedShape::FacetedShape(
    std::vector<core::Vec3> vertices,
    std::vector<core::Edge> edges,
    std::vector<FaceDefinition> faces
)
    : vertices_(std::move(vertices)),
      edges_(std::move(edges)),
      faces_(std::move(faces))
{
}

std::string FacetedShape::name() const {
    return "FacetedShape";
}

core::Mesh FacetedShape::toMesh() const {
    core::Mesh mesh;
    mesh.vertices = vertices_;
    mesh.edges = edges_;
    mesh.normals.resize(mesh.vertices.size());
    bool derive_edges = mesh.edges.empty();

    for (const auto& face : faces_) {
        if (derive_edges) {
            addFaceEdges(mesh, face);
        }

        int anchor = face.vertices.front();
        for (std::size_t index = 1; index + 1 < face.vertices.size(); ++index) {
            int b = face.vertices[index];
            int c = face.vertices[index + 1];
            core::Vec3 normal = (mesh.vertices[b] - mesh.vertices[anchor])
                .cross(mesh.vertices[c] - mesh.vertices[anchor])
                .normalized();

            mesh.triangles.push_back({anchor, b, c});
            addNormal(mesh.normals[anchor], normal);
            addNormal(mesh.normals[b], normal);
            addNormal(mesh.normals[c], normal);
        }
    }

    for (auto& normal : mesh.normals) {
        normal = normal.normalized();
    }

    return mesh;
}

} // namespace langcad::geometry
