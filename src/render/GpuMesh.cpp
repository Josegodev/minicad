#include "langcad/render/GpuMesh.hpp"

#include "langcad/render/OpenGLApi.hpp"

#include <stdexcept>
#include <utility>

namespace langcad::render {

namespace {

std::vector<float> flattenVertices(const std::vector<core::Vec3>& values) {
    std::vector<float> flattened;
    flattened.reserve(values.size() * 3);

    for (const auto& value : values) {
        flattened.push_back(static_cast<float>(value.x));
        flattened.push_back(static_cast<float>(value.y));
        flattened.push_back(static_cast<float>(value.z));
    }

    return flattened;
}

void deleteBuffer(GLuint& buffer) {
    if (buffer != 0) {
        glapi::DeleteBuffers(1, &buffer);
        buffer = 0;
    }
}

} // namespace

GpuMesh::GpuMesh(const core::Mesh& mesh)
    : vertex_buffer_(0),
      normal_buffer_(0),
      index_buffer_(0),
      index_count_(0)
{
    if (mesh.vertices.empty() || mesh.normals.size() != mesh.vertices.size() || mesh.triangles.empty()) {
        throw std::invalid_argument("GpuMesh requires vertices, matching normals and triangles");
    }

    std::vector<unsigned int> indices;
    indices.reserve(mesh.triangles.size() * 3);
    for (const auto& triangle : mesh.triangles) {
        indices.push_back(static_cast<unsigned int>(triangle.a));
        indices.push_back(static_cast<unsigned int>(triangle.b));
        indices.push_back(static_cast<unsigned int>(triangle.c));
    }

    auto vertices = flattenVertices(mesh.vertices);
    auto normals = flattenVertices(mesh.normals);
    index_count_ = static_cast<GLsizei>(indices.size());

    glapi::GenBuffers(1, &vertex_buffer_);
    glapi::BindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
    glapi::BufferData(GL_ARRAY_BUFFER, static_cast<glapi::GLsizeiptr>(vertices.size() * sizeof(float)), vertices.data(), GL_STATIC_DRAW);

    glapi::GenBuffers(1, &normal_buffer_);
    glapi::BindBuffer(GL_ARRAY_BUFFER, normal_buffer_);
    glapi::BufferData(GL_ARRAY_BUFFER, static_cast<glapi::GLsizeiptr>(normals.size() * sizeof(float)), normals.data(), GL_STATIC_DRAW);

    glapi::GenBuffers(1, &index_buffer_);
    glapi::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_);
    glapi::BufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<glapi::GLsizeiptr>(indices.size() * sizeof(unsigned int)), indices.data(), GL_STATIC_DRAW);
}

GpuMesh::~GpuMesh() {
    deleteBuffer(vertex_buffer_);
    deleteBuffer(normal_buffer_);
    deleteBuffer(index_buffer_);
}

GpuMesh::GpuMesh(GpuMesh&& other) noexcept
    : vertex_buffer_(std::exchange(other.vertex_buffer_, 0)),
      normal_buffer_(std::exchange(other.normal_buffer_, 0)),
      index_buffer_(std::exchange(other.index_buffer_, 0)),
      index_count_(std::exchange(other.index_count_, 0))
{
}

GpuMesh& GpuMesh::operator=(GpuMesh&& other) noexcept {
    if (this != &other) {
        deleteBuffer(vertex_buffer_);
        deleteBuffer(normal_buffer_);
        deleteBuffer(index_buffer_);
        vertex_buffer_ = std::exchange(other.vertex_buffer_, 0);
        normal_buffer_ = std::exchange(other.normal_buffer_, 0);
        index_buffer_ = std::exchange(other.index_buffer_, 0);
        index_count_ = std::exchange(other.index_count_, 0);
    }

    return *this;
}

void GpuMesh::draw(const ShaderProgram& shader) const {
    GLint position_location = shader.attributeLocation("a_position");
    GLint normal_location = shader.attributeLocation("a_normal");
    if (position_location < 0 || normal_location < 0) {
        return;
    }

    glapi::BindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
    glapi::EnableVertexAttribArray(static_cast<GLuint>(position_location));
    glapi::VertexAttribPointer(static_cast<GLuint>(position_location), 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glapi::BindBuffer(GL_ARRAY_BUFFER, normal_buffer_);
    glapi::EnableVertexAttribArray(static_cast<GLuint>(normal_location));
    glapi::VertexAttribPointer(static_cast<GLuint>(normal_location), 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glapi::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_);
    glDrawElements(GL_TRIANGLES, index_count_, GL_UNSIGNED_INT, nullptr);

    glapi::DisableVertexAttribArray(static_cast<GLuint>(normal_location));
    glapi::DisableVertexAttribArray(static_cast<GLuint>(position_location));
}

GpuLines::GpuLines(const std::vector<core::Vec3>& vertices, const std::vector<unsigned int>& indices)
    : vertex_buffer_(0),
      index_buffer_(0),
      index_count_(static_cast<GLsizei>(indices.size()))
{
    auto flattened = flattenVertices(vertices);

    glapi::GenBuffers(1, &vertex_buffer_);
    glapi::BindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
    glapi::BufferData(GL_ARRAY_BUFFER, static_cast<glapi::GLsizeiptr>(flattened.size() * sizeof(float)), flattened.data(), GL_STATIC_DRAW);

    glapi::GenBuffers(1, &index_buffer_);
    glapi::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_);
    glapi::BufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<glapi::GLsizeiptr>(indices.size() * sizeof(unsigned int)), indices.data(), GL_STATIC_DRAW);
}

GpuLines::~GpuLines() {
    deleteBuffer(vertex_buffer_);
    deleteBuffer(index_buffer_);
}

GpuLines::GpuLines(GpuLines&& other) noexcept
    : vertex_buffer_(std::exchange(other.vertex_buffer_, 0)),
      index_buffer_(std::exchange(other.index_buffer_, 0)),
      index_count_(std::exchange(other.index_count_, 0))
{
}

GpuLines& GpuLines::operator=(GpuLines&& other) noexcept {
    if (this != &other) {
        deleteBuffer(vertex_buffer_);
        deleteBuffer(index_buffer_);
        vertex_buffer_ = std::exchange(other.vertex_buffer_, 0);
        index_buffer_ = std::exchange(other.index_buffer_, 0);
        index_count_ = std::exchange(other.index_count_, 0);
    }

    return *this;
}

void GpuLines::draw(const ShaderProgram& shader) const {
    GLint position_location = shader.attributeLocation("a_position");
    if (position_location < 0) {
        return;
    }

    glapi::BindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
    glapi::EnableVertexAttribArray(static_cast<GLuint>(position_location));
    glapi::VertexAttribPointer(static_cast<GLuint>(position_location), 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glapi::BindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_);
    glDrawElements(GL_LINES, index_count_, GL_UNSIGNED_INT, nullptr);

    glapi::DisableVertexAttribArray(static_cast<GLuint>(position_location));
}

GpuPoints::GpuPoints(const std::vector<core::Vec3>& vertices)
    : vertex_buffer_(0),
      vertex_count_(static_cast<GLsizei>(vertices.size()))
{
    auto flattened = flattenVertices(vertices);

    glapi::GenBuffers(1, &vertex_buffer_);
    glapi::BindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
    glapi::BufferData(GL_ARRAY_BUFFER, static_cast<glapi::GLsizeiptr>(flattened.size() * sizeof(float)), flattened.data(), GL_STATIC_DRAW);
}

GpuPoints::~GpuPoints() {
    deleteBuffer(vertex_buffer_);
}

GpuPoints::GpuPoints(GpuPoints&& other) noexcept
    : vertex_buffer_(std::exchange(other.vertex_buffer_, 0)),
      vertex_count_(std::exchange(other.vertex_count_, 0))
{
}

GpuPoints& GpuPoints::operator=(GpuPoints&& other) noexcept {
    if (this != &other) {
        deleteBuffer(vertex_buffer_);
        vertex_buffer_ = std::exchange(other.vertex_buffer_, 0);
        vertex_count_ = std::exchange(other.vertex_count_, 0);
    }

    return *this;
}

void GpuPoints::draw(const ShaderProgram& shader) const {
    GLint position_location = shader.attributeLocation("a_position");
    if (position_location < 0) {
        return;
    }

    glapi::BindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
    glapi::EnableVertexAttribArray(static_cast<GLuint>(position_location));
    glapi::VertexAttribPointer(static_cast<GLuint>(position_location), 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glPointSize(7.0f);
    glDrawArrays(GL_POINTS, 0, vertex_count_);

    glapi::DisableVertexAttribArray(static_cast<GLuint>(position_location));
}

} // namespace langcad::render
