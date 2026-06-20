#pragma once

#include "langcad/core/Mesh.hpp"
#include "langcad/render/Camera.hpp"
#include "langcad/scene/Scene.hpp"

#include <SDL.h>

#include <string>

namespace langcad::render {

class Renderer {
public:
    Renderer(int width, int height, const std::string& title);
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    bool handleEvents(Camera& camera);
    void render(const Camera& camera, const scene::Scene& scene);
    void drawMesh(const core::Mesh& mesh, const Camera& camera) const;

private:
    int width_;
    int height_;
    SDL_Window* window_;
    SDL_GLContext context_;

    void loadCamera(const Camera& camera) const;
    void drawAxes() const;
    bool isValidEdge(const core::Mesh& mesh, const core::Edge& edge) const;
};

} // namespace langcad::render
