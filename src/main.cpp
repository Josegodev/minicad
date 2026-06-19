#include "Camera.hpp"
#include "Mesh.hpp"
#include "OpenGLRenderer.hpp"

#include <exception>
#include <iostream>
#include <string>

namespace {

minicad::geometry::Mesh chooseMesh() {
    std::cout << "Elige figura 3D:\n";
    std::cout << "  1) cubo\n";
    std::cout << "  2) cilindro\n";
    std::cout << "> ";

    std::string option;
    std::cin >> option;

    if (option == "2" || option == "cilindro" || option == "cylinder") {
        return minicad::geometry::Mesh::cylinder(0.7, 1.6, 32);
    }

    return minicad::geometry::Mesh::cube(1.4);
}

} // namespace

int main() {
    try {
        minicad::geometry::Mesh mesh = chooseMesh();

        minicad::graphics::OpenGLRenderer renderer(1024, 768, "MiniCAD - 3D Viewer");
        minicad::graphics::Camera camera;

        bool running = true;

        while (running) {
            running = renderer.handleEvents(camera);
            renderer.render(camera, mesh);
        }
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
