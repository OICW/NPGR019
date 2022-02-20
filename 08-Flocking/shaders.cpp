/*
 * Source code for the NPGR019 lab practices. Copyright Martin Kahoun 2021.
 * Licensed under the zlib license, see LICENSE.txt in the root directory.
 */

#include "shaders.h"

GLuint shaderProgram[ShaderProgram::NumShaderPrograms] = {0};

bool compileShaders()
{
  GLuint vertexShader[VertexShader::NumVertexShaders] = {0};
  GLuint fragmentShader[FragmentShader::NumFragmentShaders] = {0};
  GLuint computeShader[FragmentShader::NumFragmentShaders] = {0};

  // Cleanup lambda
  auto cleanUp = [&]()
  {
    // First detach shaders from programs
    GLsizei count = 0;
    GLuint shaders[2];
    for (int i = 0; i < ShaderProgram::NumShaderPrograms; ++i)
    {
      if (glIsProgram(shaderProgram[i]))
      {
        // Note: we're safe with caching up to 2 shaders, CS is standalone single stage,
        // i.e., there's either VS & FS or just CS attached to a single program
        glGetAttachedShaders(shaderProgram[i], 2, &count, shaders);
        for (GLsizei j = 0; j < count; ++j)
        {
          glDetachShader(shaderProgram[i], shaders[j]);
        }
      }
    }

    for (int i = 0; i < VertexShader::NumVertexShaders; ++i)
    {
      if (glIsShader(vertexShader[i]))
        glDeleteShader(vertexShader[i]);
    }

    for (int i = 0; i < FragmentShader::NumFragmentShaders; ++i)
    {
      if (glIsShader(fragmentShader[i]))
        glDeleteShader(fragmentShader[i]);
    }

    for (int i = 0; i < ComputeShader::NumComputeShaders; ++i)
    {
      if (glIsShader(computeShader[i]))
        glDeleteShader(computeShader[i]);
    }
  };

  // Compile all vertex shaders
  for (int i = 0; i < VertexShader::NumVertexShaders; ++i)
  {
    vertexShader[i] = ShaderCompiler::CompileShader(vsSource, i, GL_VERTEX_SHADER);
    if (!vertexShader[i])
    {
      cleanUp();
      return false;
    }
  }

  // Compile all fragment shaders
  for (int i = 0; i < FragmentShader::NumFragmentShaders; ++i)
  {
    fragmentShader[i] = ShaderCompiler::CompileShader(fsSource, i, GL_FRAGMENT_SHADER);
    if (!fragmentShader[i])
    {
      cleanUp();
      return false;
    }
  }

  // Compile all compute shaders
  for (int i = 0; i < ComputeShader::NumComputeShaders; ++i)
  {
    computeShader[i] = ShaderCompiler::CompileShader(csSource, i, GL_COMPUTE_SHADER);
    if (!computeShader[i])
    {
      cleanUp();
      return false;
    }
  }

  // Shader program for flocking simulation
  shaderProgram[ShaderProgram::Flocking] = glCreateProgram();
  glAttachShader(shaderProgram[ShaderProgram::Flocking], computeShader[ComputeShader::Flocking]);
  if (!ShaderCompiler::LinkProgram(shaderProgram[ShaderProgram::Flocking]))
  {
    cleanUp();
    return false;
  }

  // Shader program for instanced geometry w/ color
  shaderProgram[ShaderProgram::Instancing] = glCreateProgram();
  glAttachShader(shaderProgram[ShaderProgram::Instancing], vertexShader[VertexShader::Instancing]);
  glAttachShader(shaderProgram[ShaderProgram::Instancing], fragmentShader[FragmentShader::Default]);
  if (!ShaderCompiler::LinkProgram(shaderProgram[ShaderProgram::Instancing]))
  {
    cleanUp();
    return false;
  }

  // Shader program for point rendering w/ constant color
  shaderProgram[ShaderProgram::PointRendering] = glCreateProgram();
  glAttachShader(shaderProgram[ShaderProgram::PointRendering], vertexShader[VertexShader::Point]);
  glAttachShader(shaderProgram[ShaderProgram::PointRendering], fragmentShader[FragmentShader::SingleColor]);
  if (!ShaderCompiler::LinkProgram(shaderProgram[ShaderProgram::PointRendering]))
  {
    cleanUp();
    return false;
  }

  // Shader program for rendering tonemapping post-process
  shaderProgram[ShaderProgram::Tonemapping] = glCreateProgram();
  glAttachShader(shaderProgram[ShaderProgram::Tonemapping], vertexShader[VertexShader::ScreenQuad]);
  glAttachShader(shaderProgram[ShaderProgram::Tonemapping], fragmentShader[FragmentShader::Tonemapping]);
  if (!ShaderCompiler::LinkProgram(shaderProgram[ShaderProgram::Tonemapping]))
  {
    cleanUp();
    return false;
  }

  cleanUp();
  return true;
}
