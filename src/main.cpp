#include "Camera.hpp"
#include "Mesh.hpp"
#include "OpenGLRenderer.hpp"

#include <exception>
#include <iostream>

int main() {
    try {
        minicad::graphics::OpenGLRenderer renderer(1024, 768, "MiniCAD - 3D Viewer");
        minicad::graphics::Camera camera;
        minicad::geometry::Mesh cube = minicad::geometry::Mesh::cube(1.4);

        bool running = true;

        while (running) {
            running = renderer.handleEvents(camera);
            renderer.render(camera, cube);
        }
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
