#pragma once

#include "langcad/geometry/Shape3D.hpp"

#include <memory>
#include <vector>

namespace langcad::scene {

class Scene {
public:
    void add(std::unique_ptr<geometry::Shape3D> shape);

    const std::vector<std::unique_ptr<geometry::Shape3D>>& shapes() const;

private:
    std::vector<std::unique_ptr<geometry::Shape3D>> shapes_;
};

} // namespace langcad::scene
