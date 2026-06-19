#pragma once

#include "Vec3.hpp"

#include <array>
#include <vector>

namespace minicad::geometry {

class Mesh {
public:
    static Mesh cube(double size);
    static Mesh cylinder(double radius, double height, int segments);

    const std::vector<minicad::core::Vec3>& vertices() const;
    const std::vector<std::array<int, 2>>& edges() const;

private:
    std::vector<minicad::core::Vec3> vertices_;
    std::vector<std::array<int, 2>> edges_;
};

} // namespace minicad::geometry
