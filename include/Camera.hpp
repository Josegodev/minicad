#pragma once

#include "Mat4.hpp"
#include "Vec3.hpp"

namespace minicad::graphics {

class Camera {
public:
    Camera();

    void orbit(double delta_yaw, double delta_pitch);

    minicad::core::Mat4 viewMatrix() const;
    minicad::core::Mat4 projectionMatrix(double aspect) const;

private:
    minicad::core::Vec3 target_;
    minicad::core::Vec3 up_;
    double distance_;
    double yaw_;
    double pitch_;
    double fov_degrees_;

    minicad::core::Vec3 position() const;
};

} // namespace minicad::graphics
