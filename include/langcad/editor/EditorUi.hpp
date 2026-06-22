#pragma once

#include "langcad/render/Renderer.hpp"

#include <SDL.h>

namespace langcad::editor {

class EditorUi {
public:
    explicit EditorUi(render::Renderer& renderer);
    ~EditorUi();

    EditorUi(const EditorUi&) = delete;
    EditorUi& operator=(const EditorUi&) = delete;

    void processEvent(const SDL_Event& event);
    void beginFrame(render::Renderer& renderer);
    void render();
    bool wantsMouse() const;
    bool wantsKeyboard() const;
};

} // namespace langcad::editor
