#include "langcad/core/Mat4.hpp"

#include <cmath>

namespace langcad::core {

namespace {

constexpr double pi = 3.14159265358979323846;

int index(int row, int column) {
    return column * 4 + row;
}

} // namespace

Mat4::Mat4()
    : values_{}
{
}

const float* Mat4::data() const {
    return values_.data();
}

std::array<float, 16> Mat4::values() const {
    return values_;
}

Mat4 Mat4::operator*(const Mat4& other) const {
    Mat4 result;

    for (int row = 0; row < 4; ++row) {
        for (int column = 0; column < 4; ++column) {
            float value = 0.0f;

            for (int k = 0; k < 4; ++k) {
                value += values_[index(row, k)] * other.values_[index(k, column)];
            }

            result.values_[index(row, column)] = value;
        }
    }

    return result;
}

Mat4 Mat4::identity() {
    Mat4 matrix;

    matrix.values_[index(0, 0)] = 1.0f;
    matrix.values_[index(1, 1)] = 1.0f;
    matrix.values_[index(2, 2)] = 1.0f;
    matrix.values_[index(3, 3)] = 1.0f;

    return matrix;
}

Mat4 Mat4::fromColumnMajor(const std::array<float, 16>& values) {
    Mat4 matrix;
    matrix.values_ = values;
    return matrix;
}

Mat4 Mat4::perspective(double fov_degrees, double aspect, double near_plane, double far_plane) {
    Mat4 matrix;

    double fov_radians = fov_degrees * pi / 180.0;
    double f = 1.0 / std::tan(fov_radians * 0.5);

    matrix.values_[index(0, 0)] = static_cast<float>(f / aspect);
    matrix.values_[index(1, 1)] = static_cast<float>(f);
    matrix.values_[index(2, 2)] = static_cast<float>((far_plane + near_plane) / (near_plane - far_plane));
    matrix.values_[index(2, 3)] = static_cast<float>((2.0 * far_plane * near_plane) / (near_plane - far_plane));
    matrix.values_[index(3, 2)] = -1.0f;

    return matrix;
}

Mat4 Mat4::lookAt(const Vec3& eye, const Vec3& target, const Vec3& up) {
    Vec3 forward = (target - eye).normalized();
    Vec3 side = forward.cross(up).normalized();
    Vec3 camera_up = side.cross(forward);

    Mat4 matrix = Mat4::identity();

    matrix.values_[index(0, 0)] = static_cast<float>(side.x);
    matrix.values_[index(0, 1)] = static_cast<float>(side.y);
    matrix.values_[index(0, 2)] = static_cast<float>(side.z);

    matrix.values_[index(1, 0)] = static_cast<float>(camera_up.x);
    matrix.values_[index(1, 1)] = static_cast<float>(camera_up.y);
    matrix.values_[index(1, 2)] = static_cast<float>(camera_up.z);

    matrix.values_[index(2, 0)] = static_cast<float>(-forward.x);
    matrix.values_[index(2, 1)] = static_cast<float>(-forward.y);
    matrix.values_[index(2, 2)] = static_cast<float>(-forward.z);

    matrix.values_[index(0, 3)] = static_cast<float>(-side.dot(eye));
    matrix.values_[index(1, 3)] = static_cast<float>(-camera_up.dot(eye));
    matrix.values_[index(2, 3)] = static_cast<float>(forward.dot(eye));

    return matrix;
}

} // namespace langcad::core
