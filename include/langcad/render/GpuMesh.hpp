#pragma once

#include "langcad/core/Mesh.hpp"
#include "langcad/render/ShaderProgram.hpp"

#include <SDL_opengl.h>

#include <vector>

namespace langcad::render {

class GpuMesh {
public:
    explicit GpuMesh(const core::Mesh& mesh);
    ~GpuMesh();

    GpuMesh(const GpuMesh&) = delete;
    GpuMesh& operator=(const GpuMesh&) = delete;

    GpuMesh(GpuMesh&& other) noexcept;
    GpuMesh& operator=(GpuMesh&& other) noexcept;

    void draw(const ShaderProgram& shader) const;

private:
    GLuint vertex_buffer_;
    GLuint normal_buffer_;
    GLuint index_buffer_;
    GLsizei index_count_;
};

class GpuLines {
public:
    GpuLines(const std::vector<core::Vec3>& vertices, const std::vector<unsigned int>& indices);
    ~GpuLines();

    GpuLines(const GpuLines&) = delete;
    GpuLines& operator=(const GpuLines&) = delete;

    GpuLines(GpuLines&& other) noexcept;
    GpuLines& operator=(GpuLines&& other) noexcept;

    void draw(const ShaderProgram& shader) const;

private:
    GLuint vertex_buffer_;
    GLuint index_buffer_;
    GLsizei index_count_;
};

} // namespace langcad::render
