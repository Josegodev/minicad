#include "langcad/editor/EditorApp.hpp"
#include "langcad/geometry/ShapeFactory.hpp"
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
              << "5 = OCC Box\n"
              << "6 = OCC Cylinder\n"
              << "7 = OCC Sphere\n"
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

        langcad::scene::Scene scene;

        scene.add(std::move(shape));
        langcad::editor::EditorApp app(std::move(scene));
        return app.run();
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
