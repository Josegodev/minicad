#pragma once

#include "langcad/editor/EditorUi.hpp"
#include "langcad/editor/TransformGizmo.hpp"
#include "langcad/render/Camera.hpp"
#include "langcad/render/Renderer.hpp"
#include "langcad/scene/Scene.hpp"

#include <atomic>
#include <memory>
#include <string>

namespace langcad::editor {

class EditorApp {
public:
    explicit EditorApp(scene::Scene scene);
    EditorApp(scene::Scene scene, std::string title, std::shared_ptr<std::atomic_bool> stop_requested = nullptr);

    int run();

private:
    render::Renderer renderer_;
    render::Camera camera_;
    scene::Scene scene_;
    EditorUi ui_;
    TransformGizmo gizmo_;
    std::shared_ptr<std::atomic_bool> stop_requested_;

    bool handleEvents();
};

} // namespace langcad::editor
