#pragma once

#include "PixelCanvas.hpp"
#include "Vec3.hpp"

namespace minicad::graphics {

class CoordinateSystem {
public:
    explicit CoordinateSystem(double size = 1.0);

    void draw(PixelCanvas& canvas) const;

private:
    double size_;

    void drawLine(
        PixelCanvas& canvas,
        const minicad::core::Vec3& start,
        const minicad::core::Vec3& end
    ) const;

    void drawTicks(PixelCanvas& canvas) const;
};

} // namespace minicad::graphics