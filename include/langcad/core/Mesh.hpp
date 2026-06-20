#pragma once

#include "langcad/core/Vec3.hpp"

#include <vector>

namespace langcad::core {

struct Edge {
    int a;
    int b;
};

struct Mesh {
    std::vector<Vec3> vertices;
    std::vector<Edge> edges;
};

} // namespace langcad::core
