cmake_minimum_required(VERSION 3.16)

project(minicad LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(minicad
    src/main.cpp
    src/core/Vec3.cpp
    src/core/Mat4.cpp
    src/geometry/Cube.cpp
    src/geometry/Cylinder.cpp
    src/geometry/Pyramid.cpp
    src/geometry/ShapeFactory.cpp
    src/geometry/Sphere.cpp
    src/scene/Scene.cpp
    src/render/Camera.cpp
    src/render/Projector.cpp
    src/render/Renderer.cpp
)

target_include_directories(minicad PRIVATE
    include
    ${SDL2_INCLUDE_DIRS}
)

if(TARGET SDL2::SDL2)
    target_link_libraries(minicad PRIVATE SDL2::SDL2 OpenGL::GL)
else()
    target_link_libraries(minicad PRIVATE ${SDL2_LIBRARIES} OpenGL::GL)
endif()
