#pragma once

#include <SDL_opengl.h>

namespace langcad::render::glapi {

using GLchar = char;
using GLsizeiptr = long;

using GenBuffersFn = void (*)(GLsizei, GLuint*);
using BindBufferFn = void (*)(GLenum, GLuint);
using BufferDataFn = void (*)(GLenum, GLsizeiptr, const void*, GLenum);
using DeleteBuffersFn = void (*)(GLsizei, const GLuint*);
using CreateShaderFn = GLuint (*)(GLenum);
using ShaderSourceFn = void (*)(GLuint, GLsizei, const GLchar* const*, const GLint*);
using CompileShaderFn = void (*)(GLuint);
using GetShaderivFn = void (*)(GLuint, GLenum, GLint*);
using GetShaderInfoLogFn = void (*)(GLuint, GLsizei, GLsizei*, GLchar*);
using DeleteShaderFn = void (*)(GLuint);
using CreateProgramFn = GLuint (*)();
using AttachShaderFn = void (*)(GLuint, GLuint);
using LinkProgramFn = void (*)(GLuint);
using GetProgramivFn = void (*)(GLuint, GLenum, GLint*);
using GetProgramInfoLogFn = void (*)(GLuint, GLsizei, GLsizei*, GLchar*);
using DeleteProgramFn = void (*)(GLuint);
using UseProgramFn = void (*)(GLuint);
using GetUniformLocationFn = GLint (*)(GLuint, const GLchar*);
using UniformMatrix4fvFn = void (*)(GLint, GLsizei, GLboolean, const GLfloat*);
using Uniform3fFn = void (*)(GLint, GLfloat, GLfloat, GLfloat);
using Uniform1fFn = void (*)(GLint, GLfloat);
using GetAttribLocationFn = GLint (*)(GLuint, const GLchar*);
using EnableVertexAttribArrayFn = void (*)(GLuint);
using DisableVertexAttribArrayFn = void (*)(GLuint);
using VertexAttribPointerFn = void (*)(GLuint, GLint, GLenum, GLboolean, GLsizei, const void*);

extern GenBuffersFn GenBuffers;
extern BindBufferFn BindBuffer;
extern BufferDataFn BufferData;
extern DeleteBuffersFn DeleteBuffers;
extern CreateShaderFn CreateShader;
extern ShaderSourceFn ShaderSource;
extern CompileShaderFn CompileShader;
extern GetShaderivFn GetShaderiv;
extern GetShaderInfoLogFn GetShaderInfoLog;
extern DeleteShaderFn DeleteShader;
extern CreateProgramFn CreateProgram;
extern AttachShaderFn AttachShader;
extern LinkProgramFn LinkProgram;
extern GetProgramivFn GetProgramiv;
extern GetProgramInfoLogFn GetProgramInfoLog;
extern DeleteProgramFn DeleteProgram;
extern UseProgramFn UseProgram;
extern GetUniformLocationFn GetUniformLocation;
extern UniformMatrix4fvFn UniformMatrix4fv;
extern Uniform3fFn Uniform3f;
extern Uniform1fFn Uniform1f;
extern GetAttribLocationFn GetAttribLocation;
extern EnableVertexAttribArrayFn EnableVertexAttribArray;
extern DisableVertexAttribArrayFn DisableVertexAttribArray;
extern VertexAttribPointerFn VertexAttribPointer;

void loadOpenGLApi();

} // namespace langcad::render::glapi
