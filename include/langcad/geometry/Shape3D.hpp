#pragma once

#include "langcad/core/Mesh.hpp"

#include <string>

namespace langcad::geometry {

class RenderableShape {
public:
    virtual ~RenderableShape() = default;

    virtual std::string name() const = 0;
    virtual core::Mesh toMesh() const = 0;
};

// Legacy name kept while the B-Rep kernel boundary is introduced. This type is
// a renderable mesh source, not the authoritative CAD/topology model.
using Shape3D = RenderableShape;

} // namespace langcad::geometry
