#pragma once

#include "langcad/core/Mat4.hpp"
#include "langcad/geometry/Shape3D.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace langcad::scene {

struct SceneObject {
    std::uint64_t id = 0;
    std::string name;
    core::Mat4 transform = core::Mat4::identity();
    std::unique_ptr<geometry::Shape3D> shape;
    bool selected = false;
    bool visible = true;
};

class Scene {
public:
    SceneObject& add(std::unique_ptr<geometry::Shape3D> shape);

    const std::vector<SceneObject>& objects() const;
    std::vector<SceneObject>& objects();
    void clearSelection();
    SceneObject* selectObject(std::uint64_t id);
    SceneObject* selectedObject();
    const SceneObject* selectedObject() const;

private:
    std::uint64_t next_id_ = 1;
    std::vector<SceneObject> objects_;
};

} // namespace langcad::scene
