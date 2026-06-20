#pragma once

#include "langcad/core/Mat4.hpp"
#include "langcad/render/Camera.hpp"

namespace langcad::render {

class Projector {
public:
    Projector(const Camera& camera, double aspect);

    const core::Mat4& projectionMatrix() const;
    const core::Mat4& viewMatrix() const;

private:
    core::Mat4 projection_;
    core::Mat4 view_;
};

} // namespace langcad::render
