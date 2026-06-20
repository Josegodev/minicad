#pragma once

#include "langcad/core/Vec3.hpp"

#include <array>

namespace langcad::core {

class Mat4 {
public:
    Mat4();

    const float* data() const;

    static Mat4 identity();
    static Mat4 perspective(double fov_degrees, double aspect, double near_plane, double far_plane);
    static Mat4 lookAt(const Vec3& eye, const Vec3& target, const Vec3& up);

private:
    // OpenGL fixed-function expects column-major matrices.
    std::array<float, 16> values_;
};

} // namespace langcad::core
