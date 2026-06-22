#include "langcad/geometry/OcctMeshConverter.hpp"

#include <BRepMesh_IncrementalMesh.hxx>
#include <BRep_Tool.hxx>
#include <Poly_Triangulation.hxx>
#include <TopExp_Explorer.hxx>
#include <TopLoc_Location.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopAbs_Orientation.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <gp_Pnt.hxx>

#include <map>
#include <utility>

namespace langcad::geometry {

namespace {

core::Vec3 toVec3(const gp_Pnt& point) {
    return core::Vec3(point.X(), point.Y(), point.Z());
}

void addEdge(core::Mesh& mesh, int a, int b, std::map<std::pair<int, int>, bool>& seen_edges) {
    if (a > b) {
        std::swap(a, b);
    }

    const std::pair<int, int> edge(a, b);
    if (seen_edges.insert({edge, true}).second) {
        mesh.edges.push_back({a, b});
    }
}

} // namespace

core::Mesh meshFromOcctShape(const TopoDS_Shape& shape, double linear_deflection) {
    BRepMesh_IncrementalMesh mesher(shape, linear_deflection);
    mesher.Perform();

    core::Mesh mesh;
    std::map<std::pair<int, int>, bool> seen_edges;

    for (TopExp_Explorer explorer(shape, TopAbs_FACE); explorer.More(); explorer.Next()) {
        const TopoDS_Face face = TopoDS::Face(explorer.Current());
        TopLoc_Location location;
        const Handle(Poly_Triangulation) triangulation = BRep_Tool::Triangulation(face, location);

        if (triangulation.IsNull()) {
            continue;
        }

        const gp_Trsf transform = location.Transformation();
        const int vertex_offset = static_cast<int>(mesh.vertices.size());

        for (int node_index = 1; node_index <= triangulation->NbNodes(); ++node_index) {
            gp_Pnt point = triangulation->Node(node_index);
            point.Transform(transform);
            mesh.vertices.push_back(toVec3(point));
            mesh.normals.push_back(core::Vec3());
        }

        for (int triangle_index = 1; triangle_index <= triangulation->NbTriangles(); ++triangle_index) {
            int a = 0;
            int b = 0;
            int c = 0;
            triangulation->Triangle(triangle_index).Get(a, b, c);

            int ia = vertex_offset + a - 1;
            int ib = vertex_offset + b - 1;
            int ic = vertex_offset + c - 1;

            if (face.Orientation() == TopAbs_REVERSED) {
                std::swap(ib, ic);
            }

            mesh.triangles.push_back({ia, ib, ic});
            addEdge(mesh, ia, ib, seen_edges);
            addEdge(mesh, ib, ic, seen_edges);
            addEdge(mesh, ic, ia, seen_edges);

            const core::Vec3 normal = (mesh.vertices[ib] - mesh.vertices[ia])
                .cross(mesh.vertices[ic] - mesh.vertices[ia])
                .normalized();
            mesh.normals[ia] = mesh.normals[ia] + normal;
            mesh.normals[ib] = mesh.normals[ib] + normal;
            mesh.normals[ic] = mesh.normals[ic] + normal;
        }
    }

    for (core::Vec3& normal : mesh.normals) {
        normal = normal.normalized();
    }

    return mesh;
}

} // namespace langcad::geometry
