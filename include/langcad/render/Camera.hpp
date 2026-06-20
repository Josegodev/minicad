#pragma once

#include "langcad/core/Mat4.hpp"
#include "langcad/core/Vec3.hpp"

namespace langcad::render {

class Camera {
public:
    Camera();

    void orbit(double delta_yaw, double delta_pitch);

    core::Mat4 viewMatrix() const;
    core::Mat4 projectionMatrix(double aspect) const;

private:
    core::Vec3 target_;
    core::Vec3 up_;
    double distance_;
    double yaw_;
    double pitch_;
    double fov_degrees_;

    core::Vec3 position() const;
};

} // namespace langcad::render
