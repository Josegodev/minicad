#include "langcad/render/Renderer.hpp"

#include "langcad/render/OpenGLApi.hpp"

#include <SDL_opengl.h>

#include <memory>
#include <stdexcept>
#include <vector>

namespace langcad::render {

namespace {

const char* mesh_vertex_shader = R"(
#version 120
attribute vec3 a_position;
attribute vec3 a_normal;
uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;
varying vec3 v_position;
varying vec3 v_normal;

void main() {
    vec4 world_position = u_model * vec4(a_position, 1.0);
    v_position = world_position.xyz;
    v_normal = normalize(mat3(u_model) * a_normal);
    gl_Position = u_projection * u_view * world_position;
}
 )";

const char* mesh_fragment_shader = R"(
#version 120
uniform vec3 u_light_direction;
uniform vec3 u_view_position;
uniform vec3 u_base_color;
uniform float u_ambient_strength;
uniform float u_specular_strength;
uniform float u_shininess;
varying vec3 v_position;
varying vec3 v_normal;

void main() {
    vec3 normal = normalize(v_normal);
    vec3 light_direction = normalize(-u_light_direction);
    vec3 view_direction = normalize(u_view_position - v_position);
    vec3 half_direction = normalize(light_direction + view_direction);

    float diffuse = max(dot(normal, light_direction), 0.0);
    float specular = pow(max(dot(normal, half_direction), 0.0), u_shininess) * u_specular_strength;
    vec3 color = (u_ambient_strength + diffuse) * u_base_color + specular * vec3(1.0);
    gl_FragColor = vec4(color, 1.0);
}
 )";

const char* line_vertex_shader = R"(
#version 120
attribute vec3 a_position;
uniform mat4 u_view;
uniform mat4 u_projection;

void main() {
    gl_Position = u_projection * u_view * vec4(a_position, 1.0);
}
 )";

const char* line_fragment_shader = R"(
#version 120
uniform vec3 u_color;

void main() {
    gl_FragColor = vec4(u_color, 1.0);
}
 )";

} // namespace

Renderer::Renderer(int width, int height, const std::string& title)
    : width_(width),
      height_(height),
      window_(nullptr),
      context_(nullptr)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        throw std::runtime_error(SDL_GetError());
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    window_ = SDL_CreateWindow(
        title.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width_,
        height_,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );

    if (!window_) {
        throw std::runtime_error(SDL_GetError());
    }

    context_ = SDL_GL_CreateContext(window_);

    if (!context_) {
        throw std::runtime_error(SDL_GetError());
    }

    SDL_GL_SetSwapInterval(1);

    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glLineWidth(2.0f);

    initializeModernPipeline();
}

Renderer::~Renderer() {
    mesh_cache_.clear();
    axes_.reset();
    line_shader_.reset();
    mesh_shader_.reset();

    if (context_) {
        SDL_GL_DeleteContext(context_);
    }

    if (window_) {
        SDL_DestroyWindow(window_);
    }

    SDL_Quit();
}

bool Renderer::handleEvents(Camera& camera) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            return false;
        }

        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
            width_ = event.window.data1;
            height_ = event.window.data2;
        }

        if (event.type == SDL_MOUSEWHEEL) {
            camera.zoom(event.wheel.y < 0 ? 0.35 : -0.35);
        }

        if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
                return false;
            case SDLK_q:
                camera.zoom(-0.35);
                break;
            case SDLK_e:
                camera.zoom(0.35);
                break;
            case SDLK_LEFT:
            case SDLK_a:
                camera.orbit(-0.08, 0.0);
                break;
            case SDLK_RIGHT:
            case SDLK_d:
                camera.orbit(0.08, 0.0);
                break;
            case SDLK_UP:
            case SDLK_w:
                camera.orbit(0.0, 0.08);
                break;
            case SDLK_DOWN:
            case SDLK_s:
                camera.orbit(0.0, -0.08);
                break;
            default:
                break;
            }
        }
    }

    return true;
}

void Renderer::render(const Camera& camera, const scene::Scene& scene) {
    render(camera, scene, true);
}

void Renderer::render(const Camera& camera, const scene::Scene& scene, bool present_frame) {
    if (height_ <= 0) {
        height_ = 1;
    }

    glViewport(0, 0, width_, height_);
    glClearColor(0.218f, 0.218f, 0.218f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawAxes(camera);

    double aspect = height_ > 0
        ? static_cast<double>(width_) / static_cast<double>(height_)
        : 1.0;
    core::Mat4 view = camera.viewMatrix();
    core::Mat4 projection = camera.projectionMatrix(aspect);

    mesh_shader_->use();
    mesh_shader_->setMat4("u_view", view);
    mesh_shader_->setMat4("u_projection", projection);
    core::Vec3 light_direction = (core::Vec3(0.0, 0.0, 0.0) - camera.position()).normalized();
    mesh_shader_->setVec3("u_light_direction", light_direction);
    mesh_shader_->setVec3("u_view_position", camera.position());
    mesh_shader_->setVec3("u_base_color", core::Vec3(1.0, 0.0, 0.0));
    mesh_shader_->setFloat("u_ambient_strength", 0.18f);
    mesh_shader_->setFloat("u_specular_strength", 0.45f);
    mesh_shader_->setFloat("u_shininess", 32.0f);

    for (const auto& object : scene.objects()) {
        if (!object.visible || !object.shape) {
            continue;
        }

        mesh_shader_->setMat4("u_model", object.transform);
        gpuMeshFor(*object.shape).draw(*mesh_shader_);
    }

    glapi::UseProgram(0);

    if (present_frame) {
        present();
    }
}

void Renderer::present() {
    SDL_GL_SwapWindow(window_);
}

SDL_Window* Renderer::window() const {
    return window_;
}

SDL_GLContext Renderer::context() const {
    return context_;
}

int Renderer::width() const {
    return width_;
}

int Renderer::height() const {
    return height_;
}

void Renderer::resize(int width, int height) {
    width_ = width;
    height_ = height > 0 ? height : 1;
}

void Renderer::initializeModernPipeline() {
    glapi::loadOpenGLApi();
    mesh_shader_ = std::make_unique<ShaderProgram>(mesh_vertex_shader, mesh_fragment_shader);
    line_shader_ = std::make_unique<ShaderProgram>(line_vertex_shader, line_fragment_shader);

    axes_ = std::make_unique<GpuLines>(
        std::vector<core::Vec3>{
            core::Vec3(0.0, 0.0, 0.0), core::Vec3(2.0, 0.0, 0.0),
            core::Vec3(0.0, 0.0, 0.0), core::Vec3(0.0, 2.0, 0.0),
            core::Vec3(0.0, 0.0, 0.0), core::Vec3(0.0, 0.0, 2.0)
        },
        std::vector<unsigned int>{0, 1, 2, 3, 4, 5}
    );
}

void Renderer::drawAxes(const Camera& camera) const {
    double aspect = height_ > 0
        ? static_cast<double>(width_) / static_cast<double>(height_)
        : 1.0;

    line_shader_->use();
    line_shader_->setMat4("u_view", camera.viewMatrix());
    line_shader_->setMat4("u_projection", camera.projectionMatrix(aspect));
    line_shader_->setVec3("u_color", core::Vec3(0.15, 0.15, 0.15));
    axes_->draw(*line_shader_);
}

GpuMesh& Renderer::gpuMeshFor(const geometry::Shape3D& shape) {
    auto found = mesh_cache_.find(&shape);
    if (found != mesh_cache_.end()) {
        return *found->second;
    }

    auto gpu_mesh = std::make_unique<GpuMesh>(shape.toMesh());
    GpuMesh& reference = *gpu_mesh;
    mesh_cache_.emplace(&shape, std::move(gpu_mesh));
    return reference;
}

} // namespace langcad::render
