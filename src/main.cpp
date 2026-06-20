#include "langcad/geometry/ShapeFactory.hpp"
#include "langcad/render/Camera.hpp"
#include "langcad/render/Renderer.hpp"
#include "langcad/scene/Scene.hpp"

#include <exception>
#include <iostream>
#include <string>
#include <utility>

namespace {

void printMenu() {
    std::cout << "Select shape:\n"
              << "1 = Cube\n"
              << "2 = Cylinder\n"
              << "3 = Pyramid\n"
              << "4 = Sphere\n"
              << "Selection: ";
}

int readSelection(int argc, char* argv[]) {
    if (argc >= 2) {
        return std::stoi(argv[1]);
    }

    int selection = 0;
    printMenu();
    std::cin >> selection;

    return selection;
}

} // namespace

int main(int argc, char* argv[]) {
    try {
        int selection = readSelection(argc, argv);
        auto shape = langcad::geometry::createShapeFromSelection(selection);

        langcad::render::Renderer renderer(1024, 768, "MiniCAD - 3D Viewer");
        langcad::render::Camera camera;
        langcad::scene::Scene scene;

        scene.add(std::move(shape));

        bool running = true;

        while (running) {
            running = renderer.handleEvents(camera);
            renderer.render(camera, scene);
        }
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
