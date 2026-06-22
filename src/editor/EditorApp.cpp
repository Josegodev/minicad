#include "langcad/editor/EditorApp.hpp"

#include <SDL.h>

#include <utility>

namespace langcad::editor {

EditorApp::EditorApp(scene::Scene scene)
    : EditorApp(std::move(scene), "MiniCAD - CAD Editor")
{
}

EditorApp::EditorApp(scene::Scene scene, std::string title, std::shared_ptr<std::atomic_bool> stop_requested)
    : renderer_(1024, 768, title),
      camera_(),
      scene_(std::move(scene)),
      ui_(renderer_),
      gizmo_(),
      stop_requested_(std::move(stop_requested))
{
}

int EditorApp::run() {
    bool running = true;

    while (running && (!stop_requested_ || !stop_requested_->load())) {
        running = handleEvents();
        ui_.beginFrame(renderer_);

        renderer_.render(camera_, scene_, false);
        gizmo_.draw(scene_, camera_, renderer_.width(), renderer_.height());
        ui_.render();
        renderer_.present();
    }

    return 0;
}

bool EditorApp::handleEvents() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        ui_.processEvent(event);

        if (event.type == SDL_QUIT) {
            return false;
        }

        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
            renderer_.resize(event.window.data1, event.window.data2);
        }

        if (event.type == SDL_MOUSEWHEEL && !ui_.wantsMouse()) {
            camera_.zoom(event.wheel.y < 0 ? 0.35 : -0.35);
        }

        if (event.type == SDL_KEYDOWN && !ui_.wantsKeyboard()) {
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
                return false;
            case SDLK_q:
                camera_.zoom(-0.35);
                break;
            case SDLK_e:
                camera_.zoom(0.35);
                break;
            case SDLK_LEFT:
            case SDLK_a:
                camera_.orbit(-0.08, 0.0);
                break;
            case SDLK_RIGHT:
            case SDLK_d:
                camera_.orbit(0.08, 0.0);
                break;
            case SDLK_UP:
            case SDLK_w:
                camera_.orbit(0.0, 0.08);
                break;
            case SDLK_DOWN:
            case SDLK_s:
                camera_.orbit(0.0, -0.08);
                break;
            default:
                break;
            }
        }
    }

    return true;
}

} // namespace langcad::editor
