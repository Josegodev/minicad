#include "langcad/scene/Scene.hpp"

#include <stdexcept>

namespace langcad::scene {

SceneObject& Scene::add(std::unique_ptr<geometry::Shape3D> shape) {
    if (!shape) {
        throw std::invalid_argument("Scene::add received a null shape");
    }

    bool should_select = objects_.empty();
    SceneObject object;
    object.id = next_id_++;
    object.name = shape->name();
    object.shape = std::move(shape);
    object.selected = should_select;

    objects_.push_back(std::move(object));
    return objects_.back();
}

const std::vector<SceneObject>& Scene::objects() const {
    return objects_;
}

std::vector<SceneObject>& Scene::objects() {
    return objects_;
}

void Scene::clearSelection() {
    for (auto& object : objects_) {
        object.selected = false;
    }
}

SceneObject* Scene::selectObject(std::uint64_t id) {
    SceneObject* selected = nullptr;

    for (auto& object : objects_) {
        object.selected = object.id == id;
        if (object.selected) {
            selected = &object;
        }
    }

    return selected;
}

SceneObject* Scene::selectedObject() {
    for (auto& object : objects_) {
        if (object.selected) {
            return &object;
        }
    }

    return nullptr;
}

const SceneObject* Scene::selectedObject() const {
    for (const auto& object : objects_) {
        if (object.selected) {
            return &object;
        }
    }

    return nullptr;
}

} // namespace langcad::scene
