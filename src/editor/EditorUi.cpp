#include "langcad/editor/EditorUi.hpp"

#include <imgui.h>
#include <backends/imgui_impl_opengl2.h>
#include <backends/imgui_impl_sdl2.h>

namespace langcad::editor {

EditorUi::EditorUi(render::Renderer& renderer) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(renderer.window(), renderer.context());
    ImGui_ImplOpenGL2_Init();
}

EditorUi::~EditorUi() {
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void EditorUi::processEvent(const SDL_Event& event) {
    ImGui_ImplSDL2_ProcessEvent(&event);
}

void EditorUi::beginFrame(render::Renderer& renderer) {
    (void)renderer;
    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

void EditorUi::render() {
    ImGui::Render();
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
}

bool EditorUi::wantsMouse() const {
    return ImGui::GetIO().WantCaptureMouse;
}

bool EditorUi::wantsKeyboard() const {
    return ImGui::GetIO().WantCaptureKeyboard;
}

} // namespace langcad::editor
