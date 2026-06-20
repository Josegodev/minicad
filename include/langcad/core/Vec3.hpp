#pragma once

namespace langcad::core {

struct Vec3 {
    double x;
    double y;
    double z;

    Vec3();
    Vec3(double x_value, double y_value, double z_value);

    Vec3 operator+(const Vec3& other) const;
    Vec3 operator-(const Vec3& other) const;
    Vec3 operator*(double scalar) const;
    Vec3 operator/(double scalar) const;

    double dot(const Vec3& other) const;
    Vec3 cross(const Vec3& other) const;

    double norm() const;
    Vec3 normalized() const;
};

} // namespace langcad::core
