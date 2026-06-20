#pragma once

#include "langcad/core/Mesh.hpp"

#include <string>

namespace langcad::geometry {

class Shape3D {
public:
    virtual ~Shape3D() = default;

    virtual std::string name() const = 0;
    virtual core::Mesh toMesh() const = 0;
};

} // namespace langcad::geometry
