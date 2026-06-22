#pragma once

#include "langcad/core/Mesh.hpp"
#include "langcad/geometry/Shape3D.hpp"
#include "langcad/render/Camera.hpp"
#include "langcad/render/GpuMesh.hpp"
#include "langcad/render/ShaderProgram.hpp"
#include "langcad/scene/Scene.hpp"

#include <SDL.h>

#include <memory>
#include <string>
#include <unordered_map>

namespace langcad::render {

class Renderer {
public:
    Renderer(int width, int height, const std::string& title);
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    bool handleEvents(Camera& camera);
    void render(const Camera& camera, const scene::Scene& scene);
    void render(const Camera& camera, const scene::Scene& scene, bool present);
    void present();
    SDL_Window* window() const;
    SDL_GLContext context() const;
    int width() const;
    int height() const;
    void resize(int width, int height);

private:
    int width_;
    int height_;
    SDL_Window* window_;
    SDL_GLContext context_;
    std::unique_ptr<ShaderProgram> mesh_shader_;
    std::unique_ptr<ShaderProgram> line_shader_;
    std::unique_ptr<GpuLines> axes_;
    std::unordered_map<const geometry::Shape3D*, std::unique_ptr<GpuMesh>> mesh_cache_;

    void initializeModernPipeline();
    void drawAxes(const Camera& camera) const;
    GpuMesh& gpuMeshFor(const geometry::Shape3D& shape);
};

} // namespace langcad::render
