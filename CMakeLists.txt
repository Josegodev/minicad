cmake_minimum_required(VERSION 3.16)

project(minicad LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(OpenCASCADE REQUIRED)

set(LANGCAD_OCC_LIBRARIES
    /usr/lib/x86_64-linux-gnu/libTKPrim.so.7.6.3
    /usr/lib/x86_64-linux-gnu/libTKMesh.so.7.6.3
    /usr/lib/x86_64-linux-gnu/libTKShHealing.so.7.6.3
    /usr/lib/x86_64-linux-gnu/libTKTopAlgo.so.7.6.3
    /usr/lib/x86_64-linux-gnu/libTKGeomAlgo.so.7.6.3
    /usr/lib/x86_64-linux-gnu/libTKBRep.so.7.6.3
    /usr/lib/x86_64-linux-gnu/libTKGeomBase.so.7.6.3
    /usr/lib/x86_64-linux-gnu/libTKG3d.so.7.6.3
    /usr/lib/x86_64-linux-gnu/libTKG2d.so.7.6.3
    /usr/lib/x86_64-linux-gnu/libTKMath.so.7.6.3
    /usr/lib/x86_64-linux-gnu/libTKernel.so.7.6.3
    /usr/lib/x86_64-linux-gnu/libtbb.so.12
    /usr/lib/x86_64-linux-gnu/libtbbmalloc.so.2
)

FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_Declare(
    cpp_httplib
    URL https://github.com/yhirose/cpp-httplib/archive/refs/tags/v0.18.6.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.9
)

FetchContent_Declare(
    imguizmo
    GIT_REPOSITORY https://github.com/CedricGuillemet/ImGuizmo.git
    GIT_TAG master
)

FetchContent_MakeAvailable(nlohmann_json cpp_httplib imgui)

FetchContent_GetProperties(imguizmo)
if(NOT imguizmo_POPULATED)
    FetchContent_Populate(imguizmo)
endif()

add_library(imgui_lib STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl2.cpp
)

target_include_directories(imgui_lib PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${SDL2_INCLUDE_DIRS}
)

if(TARGET SDL2::SDL2)
    target_link_libraries(imgui_lib PUBLIC SDL2::SDL2 OpenGL::GL)
else()
    target_link_libraries(imgui_lib PUBLIC ${SDL2_LIBRARIES} OpenGL::GL)
endif()

add_library(imguizmo_lib STATIC
    ${imguizmo_SOURCE_DIR}/src/ImGuizmo.cpp
)

target_include_directories(imguizmo_lib PUBLIC
    ${imguizmo_SOURCE_DIR}/src
    ${imgui_SOURCE_DIR}
)

target_link_libraries(imguizmo_lib PUBLIC imgui_lib)

add_library(langcad_kernel
    src/core/Vec3.cpp
    src/core/Mat4.cpp
    src/geometry/Cube.cpp
    src/geometry/Cylinder.cpp
    src/geometry/FacetedShape.cpp
    src/geometry/Pyramid.cpp
    src/geometry/ShapeCatalog.cpp
    src/geometry/ShapeValidator.cpp
    src/geometry/SkeletonResolver.cpp
    src/geometry/Sphere.cpp
    src/scene/Scene.cpp
)

target_include_directories(langcad_kernel PUBLIC
    include
)

add_library(langcad_core
    src/api/ApiServer.cpp
    src/api/ShapeJson.cpp
    src/api/StructuredLogger.cpp
    src/api/Trace.cpp
    src/geometry/OcctBox.cpp
    src/geometry/OcctCylinder.cpp
    src/geometry/OcctMeshConverter.cpp
    src/geometry/OcctSphere.cpp
    src/geometry/ShapeFactory.cpp
    src/llm/OllamaClient.cpp
    src/render/Camera.cpp
    src/render/GpuMesh.cpp
    src/render/OpenGLApi.cpp
    src/render/Projector.cpp
    src/render/Renderer.cpp
    src/render/ShaderProgram.cpp
)

add_library(langcad_editor
    src/editor/EditorApp.cpp
    src/editor/EditorUi.cpp
    src/editor/TransformGizmo.cpp
)

target_include_directories(langcad_editor PUBLIC
    include
)

target_link_libraries(langcad_editor PUBLIC langcad_core imgui_lib imguizmo_lib)

target_include_directories(langcad_core PUBLIC
    include
    ${SDL2_INCLUDE_DIRS}
    ${OpenCASCADE_INCLUDE_DIR}
)

if(TARGET SDL2::SDL2)
    target_link_libraries(langcad_core PUBLIC langcad_kernel SDL2::SDL2 OpenGL::GL nlohmann_json::nlohmann_json httplib::httplib imgui_lib imguizmo_lib ${LANGCAD_OCC_LIBRARIES})
else()
    target_link_libraries(langcad_core PUBLIC langcad_kernel ${SDL2_LIBRARIES} OpenGL::GL nlohmann_json::nlohmann_json httplib::httplib imgui_lib imguizmo_lib ${LANGCAD_OCC_LIBRARIES})
endif()

add_executable(minicad
    src/main.cpp
)

target_link_libraries(minicad PRIVATE langcad_editor)

add_executable(langcad_api
    src/api_main.cpp
)

target_link_libraries(langcad_api PRIVATE langcad_editor langcad_core langcad_editor)
