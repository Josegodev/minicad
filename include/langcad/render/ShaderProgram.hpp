#pragma once

#include "langcad/core/Mat4.hpp"
#include "langcad/core/Vec3.hpp"

#include <SDL_opengl.h>

#include <string>

namespace langcad::render {

class ShaderProgram {
public:
    ShaderProgram(const char* vertex_source, const char* fragment_source);
    ~ShaderProgram();

    ShaderProgram(const ShaderProgram&) = delete;
    ShaderProgram& operator=(const ShaderProgram&) = delete;

    ShaderProgram(ShaderProgram&& other) noexcept;
    ShaderProgram& operator=(ShaderProgram&& other) noexcept;

    void use() const;
    GLint attributeLocation(const char* name) const;
    void setMat4(const char* name, const core::Mat4& value) const;
    void setVec3(const char* name, const core::Vec3& value) const;
    void setFloat(const char* name, float value) const;

private:
    GLuint id_;

    static GLuint compileShader(GLenum type, const char* source);
};

} // namespace langcad::render
