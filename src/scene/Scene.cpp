#include "langcad/scene/Scene.hpp"

#include <stdexcept>

namespace langcad::scene {

void Scene::add(std::unique_ptr<geometry::Shape3D> shape) {
    if (!shape) {
        throw std::invalid_argument("Scene::add received a null shape");
    }

    shapes_.push_back(std::move(shape));
}

const std::vector<std::unique_ptr<geometry::Shape3D>>& Scene::shapes() const {
    return shapes_;
}

} // namespace langcad::scene
