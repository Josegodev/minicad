#include "Vec3.hpp"

#include <cmath>

namespace minicad::core {

Vec3::Vec3()
    : x(0.0), y(0.0), z(0.0)
{
}

Vec3::Vec3(double x_value, double y_value, double z_value)
    : x(x_value), y(y_value), z(z_value)
{
}

Vec3 Vec3::operator+(const Vec3& other) const {
    return Vec3(
        x + other.x,
        y + other.y,
        z + other.z
    );
}

Vec3 Vec3::operator-(const Vec3& other) const {
    return Vec3(
        x - other.x,
        y - other.y,
        z - other.z
    );
}

Vec3 Vec3::operator*(double scalar) const {
    return Vec3(
        x * scalar,
        y * scalar,
        z * scalar
    );
}

Vec3 Vec3::operator/(double scalar) const {
    return Vec3(
        x / scalar,
        y / scalar,
        z / scalar
    );
}

double Vec3::dot(const Vec3& other) const {
    return x * other.x + y * other.y + z * other.z;
}

Vec3 Vec3::cross(const Vec3& other) const {
    return Vec3(
        y * other.z - z * other.y,
        z * other.x - x * other.z,
        x * other.y - y * other.x
    );
}

double Vec3::norm() const {
    return std::sqrt(x * x + y * y + z * z);
}

Vec3 Vec3::normalized() const {
    double length = norm();

    if (length == 0.0) {
        return Vec3();
    }

    return *this / length;
}

} // namespace minicad::core
