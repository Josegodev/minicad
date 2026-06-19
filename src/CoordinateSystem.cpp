#include "CoordinateSystem.hpp"

namespace minicad::graphics {

using minicad::core::Vec3;

CoordinateSystem::CoordinateSystem(double size)
    : size_(size)
{
}

void CoordinateSystem::draw(PixelCanvas& canvas) const {
    Vec3 origin(0.0, 0.0, 0.0);

    Vec3 x_negative(-size_, 0.0, 0.0);
    Vec3 x_positive( size_, 0.0, 0.0);

    Vec3 y_negative(0.0, -size_, 0.0);
    Vec3 y_positive(0.0,  size_, 0.0);

    drawLine(canvas, x_negative, x_positive);
    drawLine(canvas, y_negative, y_positive);

    canvas.putNormCentered(origin.x, origin.y);

    drawTicks(canvas);
}

void CoordinateSystem::drawLine(
    PixelCanvas& canvas,
    const Vec3& start,
    const Vec3& end
) const {
    constexpr int steps = 500;

    Vec3 direction = end - start;

    for (int i = 0; i <= steps; ++i) {
        double t = static_cast<double>(i) / static_cast<double>(steps);

        Vec3 point = start + direction * t;

        canvas.putNormCentered(point.x, point.y);
    }
}

void CoordinateSystem::drawTicks(PixelCanvas& canvas) const {
    constexpr double tick_size = 0.03;
    constexpr double spacing = 0.25;

    for (double value = -size_; value <= size_; value += spacing) {
        Vec3 x_tick_start(value, -tick_size, 0.0);
        Vec3 x_tick_end(value,  tick_size, 0.0);

        Vec3 y_tick_start(-tick_size, value, 0.0);
        Vec3 y_tick_end( tick_size, value, 0.0);

        drawLine(canvas, x_tick_start, x_tick_end);
        drawLine(canvas, y_tick_start, y_tick_end);
    }
}

} // namespace minicad::graphics