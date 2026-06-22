#include "langcad/render/ShaderProgram.hpp"

#include "langcad/render/OpenGLApi.hpp"

#include <vector>
#include <stdexcept>
#include <utility>

namespace langcad::render {

ShaderProgram::ShaderProgram(const char* vertex_source, const char* fragment_source)
    : id_(0)
{
    GLuint vertex_shader = compileShader(GL_VERTEX_SHADER, vertex_source);
    GLuint fragment_shader = compileShader(GL_FRAGMENT_SHADER, fragment_source);

    id_ = glapi::CreateProgram();
    glapi::AttachShader(id_, vertex_shader);
    glapi::AttachShader(id_, fragment_shader);
    glapi::LinkProgram(id_);

    GLint success = 0;
    glapi::GetProgramiv(id_, GL_LINK_STATUS, &success);
    if (!success) {
        GLint length = 0;
        glapi::GetProgramiv(id_, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> log(static_cast<std::size_t>(length) + 1);
        glapi::GetProgramInfoLog(id_, length, nullptr, log.data());
        glapi::DeleteShader(vertex_shader);
        glapi::DeleteShader(fragment_shader);
        throw std::runtime_error(std::string("OpenGL program link failed: ") + log.data());
    }

    glapi::DeleteShader(vertex_shader);
    glapi::DeleteShader(fragment_shader);
}

ShaderProgram::~ShaderProgram() {
    if (id_ != 0) {
        glapi::DeleteProgram(id_);
    }
}

ShaderProgram::ShaderProgram(ShaderProgram&& other) noexcept
    : id_(std::exchange(other.id_, 0))
{
}

ShaderProgram& ShaderProgram::operator=(ShaderProgram&& other) noexcept {
    if (this != &other) {
        if (id_ != 0) {
            glapi::DeleteProgram(id_);
        }
        id_ = std::exchange(other.id_, 0);
    }

    return *this;
}

void ShaderProgram::use() const {
    glapi::UseProgram(id_);
}

GLint ShaderProgram::attributeLocation(const char* name) const {
    return glapi::GetAttribLocation(id_, name);
}

void ShaderProgram::setMat4(const char* name, const core::Mat4& value) const {
    GLint location = glapi::GetUniformLocation(id_, name);
    if (location >= 0) {
        glapi::UniformMatrix4fv(location, 1, GL_FALSE, value.data());
    }
}

void ShaderProgram::setVec3(const char* name, const core::Vec3& value) const {
    GLint location = glapi::GetUniformLocation(id_, name);
    if (location >= 0) {
        glapi::Uniform3f(location, static_cast<GLfloat>(value.x), static_cast<GLfloat>(value.y), static_cast<GLfloat>(value.z));
    }
}

void ShaderProgram::setFloat(const char* name, float value) const {
    GLint location = glapi::GetUniformLocation(id_, name);
    if (location >= 0) {
        glapi::Uniform1f(location, value);
    }
}

GLuint ShaderProgram::compileShader(GLenum type, const char* source) {
    GLuint shader = glapi::CreateShader(type);
    glapi::ShaderSource(shader, 1, &source, nullptr);
    glapi::CompileShader(shader);

    GLint success = 0;
    glapi::GetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint length = 0;
        glapi::GetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> log(static_cast<std::size_t>(length) + 1);
        glapi::GetShaderInfoLog(shader, length, nullptr, log.data());
        glapi::DeleteShader(shader);
        throw std::runtime_error(std::string("OpenGL shader compile failed: ") + log.data());
    }

    return shader;
}

} // namespace langcad::render
