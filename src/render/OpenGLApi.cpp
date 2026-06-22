#include "langcad/render/OpenGLApi.hpp"

#include <SDL.h>

#include <stdexcept>

namespace langcad::render::glapi {

GenBuffersFn GenBuffers = nullptr;
BindBufferFn BindBuffer = nullptr;
BufferDataFn BufferData = nullptr;
DeleteBuffersFn DeleteBuffers = nullptr;
CreateShaderFn CreateShader = nullptr;
ShaderSourceFn ShaderSource = nullptr;
CompileShaderFn CompileShader = nullptr;
GetShaderivFn GetShaderiv = nullptr;
GetShaderInfoLogFn GetShaderInfoLog = nullptr;
DeleteShaderFn DeleteShader = nullptr;
CreateProgramFn CreateProgram = nullptr;
AttachShaderFn AttachShader = nullptr;
LinkProgramFn LinkProgram = nullptr;
GetProgramivFn GetProgramiv = nullptr;
GetProgramInfoLogFn GetProgramInfoLog = nullptr;
DeleteProgramFn DeleteProgram = nullptr;
UseProgramFn UseProgram = nullptr;
GetUniformLocationFn GetUniformLocation = nullptr;
UniformMatrix4fvFn UniformMatrix4fv = nullptr;
Uniform3fFn Uniform3f = nullptr;
Uniform1fFn Uniform1f = nullptr;
GetAttribLocationFn GetAttribLocation = nullptr;
EnableVertexAttribArrayFn EnableVertexAttribArray = nullptr;
DisableVertexAttribArrayFn DisableVertexAttribArray = nullptr;
VertexAttribPointerFn VertexAttribPointer = nullptr;

namespace {

template <typename Function>
Function load(const char* name) {
    void* pointer = SDL_GL_GetProcAddress(name);
    if (!pointer) {
        throw std::runtime_error(std::string("Missing OpenGL function: ") + name);
    }

    return reinterpret_cast<Function>(pointer);
}

} // namespace

void loadOpenGLApi() {
    static bool loaded = false;
    if (loaded) {
        return;
    }

    GenBuffers = load<GenBuffersFn>("glGenBuffers");
    BindBuffer = load<BindBufferFn>("glBindBuffer");
    BufferData = load<BufferDataFn>("glBufferData");
    DeleteBuffers = load<DeleteBuffersFn>("glDeleteBuffers");
    CreateShader = load<CreateShaderFn>("glCreateShader");
    ShaderSource = load<ShaderSourceFn>("glShaderSource");
    CompileShader = load<CompileShaderFn>("glCompileShader");
    GetShaderiv = load<GetShaderivFn>("glGetShaderiv");
    GetShaderInfoLog = load<GetShaderInfoLogFn>("glGetShaderInfoLog");
    DeleteShader = load<DeleteShaderFn>("glDeleteShader");
    CreateProgram = load<CreateProgramFn>("glCreateProgram");
    AttachShader = load<AttachShaderFn>("glAttachShader");
    LinkProgram = load<LinkProgramFn>("glLinkProgram");
    GetProgramiv = load<GetProgramivFn>("glGetProgramiv");
    GetProgramInfoLog = load<GetProgramInfoLogFn>("glGetProgramInfoLog");
    DeleteProgram = load<DeleteProgramFn>("glDeleteProgram");
    UseProgram = load<UseProgramFn>("glUseProgram");
    GetUniformLocation = load<GetUniformLocationFn>("glGetUniformLocation");
    UniformMatrix4fv = load<UniformMatrix4fvFn>("glUniformMatrix4fv");
    Uniform3f = load<Uniform3fFn>("glUniform3f");
    Uniform1f = load<Uniform1fFn>("glUniform1f");
    GetAttribLocation = load<GetAttribLocationFn>("glGetAttribLocation");
    EnableVertexAttribArray = load<EnableVertexAttribArrayFn>("glEnableVertexAttribArray");
    DisableVertexAttribArray = load<DisableVertexAttribArrayFn>("glDisableVertexAttribArray");
    VertexAttribPointer = load<VertexAttribPointerFn>("glVertexAttribPointer");

    loaded = true;
}

} // namespace langcad::render::glapi
