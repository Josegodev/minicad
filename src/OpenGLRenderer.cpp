#include "OpenGLRenderer.hpp"

#include <SDL_opengl.h>

#include <stdexcept>

namespace minicad::graphics {

OpenGLRenderer::OpenGLRenderer(int width, int height, const std::string& title)
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
}

OpenGLRenderer::~OpenGLRenderer() {
    if (context_) {
        SDL_GL_DeleteContext(context_);
    }

    if (window_) {
        SDL_DestroyWindow(window_);
    }

    SDL_Quit();
}

bool OpenGLRenderer::handleEvents(Camera& camera) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            return false;
        }

        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
            width_ = event.window.data1;
            height_ = event.window.data2;
        }

        if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
                return false;
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

void OpenGLRenderer::render(const Camera& camera, const minicad::geometry::Mesh& mesh) {
    if (height_ <= 0) {
        height_ = 1;
    }

    double aspect = static_cast<double>(width_) / static_cast<double>(height_);
    minicad::core::Mat4 projection = camera.projectionMatrix(aspect);
    minicad::core::Mat4 view = camera.viewMatrix();

    glViewport(0, 0, width_, height_);
    glClearColor(0.94f, 0.94f, 0.92f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(projection.data());

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(view.data());

    drawAxes();
    drawMeshWireframe(mesh);

    SDL_GL_SwapWindow(window_);
}

void OpenGLRenderer::drawAxes() const {
    glBegin(GL_LINES);

    glColor3f(0.9f, 0.05f, 0.05f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(2.0f, 0.0f, 0.0f);

    glColor3f(0.05f, 0.65f, 0.1f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 2.0f, 0.0f);

    glColor3f(0.05f, 0.2f, 0.9f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 2.0f);

    glEnd();
}

void OpenGLRenderer::drawMeshWireframe(const minicad::geometry::Mesh& mesh) const {
    const auto& vertices = mesh.vertices();

    glColor3f(0.05f, 0.05f, 0.05f);
    glBegin(GL_LINES);

    for (const auto& edge : mesh.edges()) {
        const auto& a = vertices[edge[0]];
        const auto& b = vertices[edge[1]];

        glVertex3d(a.x, a.y, a.z);
        glVertex3d(b.x, b.y, b.z);
    }

    glEnd();
}

} // namespace minicad::graphics
