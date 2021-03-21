/*
 * Source code for the NPGR019 lab practices. Copyright Martin Kahoun 2021.
 * Licensed under the zlib license, see LICENSE.txt in the root directory.
 */

#include <cstdio>
#include <ShaderCompiler.h>

GLuint ShaderCompiler::CompileShader(const char* source[], int index, GLenum type)
{
  // Create and compile the shader
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, source + index, nullptr);
  glCompileShader(shader);

  // Check that compilation was a success
  GLint status = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
  if (status == GL_FALSE)
  {
    char log[MAX_LOG_LENGTH];
    glGetShaderInfoLog(shader, MAX_LOG_LENGTH, nullptr, log);
    printf("Shader compilation (%d) failed: %s\n", index, log);
    return 0;
  }

  return shader;
}

bool ShaderCompiler::LinkProgram(GLuint program)
{
  glLinkProgram(program);

  // Check that linkage was a success
  GLint status = 0;
  glGetProgramiv(program, GL_LINK_STATUS, &status);
  if (status == GL_FALSE)
  {
    char log[MAX_LOG_LENGTH];
    glGetProgramInfoLog(program, MAX_LOG_LENGTH, nullptr, log);
    printf("Shader program linking failed: %s\n", log);
    return false;
  }

  return true;
}
