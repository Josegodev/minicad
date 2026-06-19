#include "Camera.hpp"

#include <algorithm>
#include <cmath>

namespace minicad::graphics {

namespace {

constexpr double pi = 3.14159265358979323846;

} // namespace

Camera::Camera()
    : target_(0.0, 0.0, 0.0),
      up_(0.0, 1.0, 0.0),
      distance_(4.0),
      yaw_(pi * 0.25),
      pitch_(pi * 0.18),
      fov_degrees_(60.0)
{
}

void Camera::orbit(double delta_yaw, double delta_pitch) {
    yaw_ += delta_yaw;
    pitch_ = std::clamp(pitch_ + delta_pitch, -1.35, 1.35);
}

minicad::core::Mat4 Camera::viewMatrix() const {
    return minicad::core::Mat4::lookAt(position(), target_, up_);
}

minicad::core::Mat4 Camera::projectionMatrix(double aspect) const {
    return minicad::core::Mat4::perspective(fov_degrees_, aspect, 0.1, 100.0);
}

minicad::core::Vec3 Camera::position() const {
    double cos_pitch = std::cos(pitch_);

    return minicad::core::Vec3(
        target_.x + distance_ * cos_pitch * std::sin(yaw_),
        target_.y + distance_ * std::sin(pitch_),
        target_.z + distance_ * cos_pitch * std::cos(yaw_)
    );
}

} // namespace minicad::graphics
