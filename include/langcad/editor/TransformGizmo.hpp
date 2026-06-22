#pragma once

#include "langcad/render/Camera.hpp"
#include "langcad/scene/Scene.hpp"

#include <imgui.h>
#include <ImGuizmo.h>

namespace langcad::editor {

class TransformGizmo {
public:
    void draw(scene::Scene& scene, const render::Camera& camera, int width, int height);

private:
    ImGuizmo::OPERATION operation_ = ImGuizmo::TRANSLATE;
    ImGuizmo::MODE mode_ = ImGuizmo::WORLD;
};

} // namespace langcad::editor
