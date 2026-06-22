#pragma once

#include "langcad/core/Vec3.hpp"

#include <vector>

namespace langcad::core {

struct Edge {
    int a;
    int b;
};

struct Triangle {
    int a;
    int b;
    int c;
};

struct Mesh {
    std::vector<Vec3> vertices;
    std::vector<Vec3> normals;
    std::vector<Edge> edges;
    std::vector<Triangle> triangles;
};

} // namespace langcad::core
