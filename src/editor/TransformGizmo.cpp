#include "langcad/editor/TransformGizmo.hpp"

#include "langcad/geometry/ShapeFactory.hpp"

#include <imgui.h>

#include <string>

namespace langcad::editor {

void TransformGizmo::draw(scene::Scene& scene, const render::Camera& camera, int width, int height) {
    ImGui::Begin("CAD Tools");

    if (ImGui::CollapsingHeader("Create", ImGuiTreeNodeFlags_DefaultOpen)) {
        auto addShape = [&scene](int selection) {
            auto& object = scene.add(geometry::createShapeFromSelection(selection));
            scene.selectObject(object.id);
        };

        if (ImGui::Button("Cube")) {
            addShape(1);
        }
        ImGui::SameLine();
        if (ImGui::Button("Cylinder")) {
            addShape(2);
        }
        ImGui::SameLine();
        if (ImGui::Button("Pyramid")) {
            addShape(3);
        }
        ImGui::SameLine();
        if (ImGui::Button("Sphere")) {
            addShape(4);
        }
    }

    if (ImGui::CollapsingHeader("Scene", ImGuiTreeNodeFlags_DefaultOpen)) {
        for (auto& object : scene.objects()) {
            ImGui::PushID(static_cast<int>(object.id));

            bool visible = object.visible;
            if (ImGui::Checkbox("##visible", &visible)) {
                object.visible = visible;
            }

            ImGui::SameLine();
            std::string label = object.name + " #" + std::to_string(object.id);
            if (ImGui::Selectable(label.c_str(), object.selected)) {
                scene.selectObject(object.id);
            }

            ImGui::PopID();
        }
    }

    ImGui::Separator();

    if (ImGui::RadioButton("Translate", operation_ == ImGuizmo::TRANSLATE)) {
        operation_ = ImGuizmo::TRANSLATE;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Rotate", operation_ == ImGuizmo::ROTATE)) {
        operation_ = ImGuizmo::ROTATE;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Scale", operation_ == ImGuizmo::SCALE)) {
        operation_ = ImGuizmo::SCALE;
    }

    if (ImGui::RadioButton("World", mode_ == ImGuizmo::WORLD)) {
        mode_ = ImGuizmo::WORLD;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Local", mode_ == ImGuizmo::LOCAL)) {
        mode_ = ImGuizmo::LOCAL;
    }

    scene::SceneObject* selected = scene.selectedObject();
    if (selected) {
        ImGui::Text("Selected: %s #%llu", selected->name.c_str(), static_cast<unsigned long long>(selected->id));
    } else {
        ImGui::TextUnformatted("No object selected");
    }

    ImGui::End();

    if (!selected) {
        return;
    }

    double aspect = height > 0
        ? static_cast<double>(width) / static_cast<double>(height)
        : 1.0;
    auto view = camera.viewMatrix().values();
    auto projection = camera.projectionMatrix(aspect).values();
    auto model = selected->transform.values();

    ImGuizmo::BeginFrame();
    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));

    if (ImGuizmo::Manipulate(view.data(), projection.data(), operation_, mode_, model.data())) {
        selected->transform = core::Mat4::fromColumnMajor(model);
    }
}

} // namespace langcad::editor
