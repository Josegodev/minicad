#include "langcad/render/Projector.hpp"

namespace langcad::render {

Projector::Projector(const Camera& camera, double aspect)
    : projection_(camera.projectionMatrix(aspect)),
      view_(camera.viewMatrix())
{
}

const core::Mat4& Projector::projectionMatrix() const {
    return projection_;
}

const core::Mat4& Projector::viewMatrix() const {
    return view_;
}

} // namespace langcad::render
