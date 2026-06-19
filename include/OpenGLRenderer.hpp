#pragma once

#include "Camera.hpp"
#include "Mesh.hpp"

#include <SDL.h>

#include <string>

namespace minicad::graphics {

class OpenGLRenderer {
public:
    OpenGLRenderer(int width, int height, const std::string& title);
    ~OpenGLRenderer();

    OpenGLRenderer(const OpenGLRenderer&) = delete;
    OpenGLRenderer& operator=(const OpenGLRenderer&) = delete;

    bool handleEvents(Camera& camera);
    void render(const Camera& camera, const minicad::geometry::Mesh& mesh);

private:
    int width_;
    int height_;
    SDL_Window* window_;
    SDL_GLContext context_;

    void drawAxes() const;
    void drawMeshWireframe(const minicad::geometry::Mesh& mesh) const;
};

} // namespace minicad::graphics
