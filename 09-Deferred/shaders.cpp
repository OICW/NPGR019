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
  };

  // UBO explicit binding lambda - call after program linking
  auto uniformBlockBinding = [](GLuint program, const char* blockName = "TransformBlock", GLint binding = 0)
  {
    // Get UBO index from the program
    GLuint uboIndex = glGetUniformBlockIndex(program, blockName);
    // Bind it always to slot "binding" - since GLSL 420, it's possible to specify it in the layout block
    glUniformBlockBinding(program, uboIndex, binding);
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

  // Shader program for non-instanced geometry writing into the GBuffer
  shaderProgram[ShaderProgram::DefaultGBuffer] = glCreateProgram();
  glAttachShader(shaderProgram[ShaderProgram::DefaultGBuffer], vertexShader[VertexShader::Default]);
  glAttachShader(shaderProgram[ShaderProgram::DefaultGBuffer], fragmentShader[FragmentShader::GBuffer]);
  if (!ShaderCompiler::LinkProgram(shaderProgram[ShaderProgram::DefaultGBuffer]))
  {
    cleanUp();
    return false;
  }
  uniformBlockBinding(shaderProgram[ShaderProgram::DefaultGBuffer]);

  // Shader program for instanced geometry writing into the GBuffer
  shaderProgram[ShaderProgram::InstancedGBuffer] = glCreateProgram();
  glAttachShader(shaderProgram[ShaderProgram::InstancedGBuffer], vertexShader[VertexShader::Instancing]);
  glAttachShader(shaderProgram[ShaderProgram::InstancedGBuffer], fragmentShader[FragmentShader::GBuffer]);
  if (!ShaderCompiler::LinkProgram(shaderProgram[ShaderProgram::InstancedGBuffer]))
  {
    cleanUp();
    return false;
  }
  uniformBlockBinding(shaderProgram[ShaderProgram::InstancedGBuffer]);
  uniformBlockBinding(shaderProgram[ShaderProgram::InstancedGBuffer], "InstanceBuffer", 1);

  // Shader program for ambient fullscreen light pass
  shaderProgram[ShaderProgram::AmbientLightPass] = glCreateProgram();
  glAttachShader(shaderProgram[ShaderProgram::AmbientLightPass], vertexShader[VertexShader::ScreenQuad]);
  glAttachShader(shaderProgram[ShaderProgram::AmbientLightPass], fragmentShader[FragmentShader::AmbientPass]);
  if (!ShaderCompiler::LinkProgram(shaderProgram[ShaderProgram::AmbientLightPass]))
  {
    cleanUp();
    return false;
  }

  // Shader program for light pass
  shaderProgram[ShaderProgram::InstancedLightPass] = glCreateProgram();
  glAttachShader(shaderProgram[ShaderProgram::InstancedLightPass], vertexShader[VertexShader::Light]);
  glAttachShader(shaderProgram[ShaderProgram::InstancedLightPass], fragmentShader[FragmentShader::LightPass]);
  if (!ShaderCompiler::LinkProgram(shaderProgram[ShaderProgram::InstancedLightPass]))
  {
    cleanUp();
    return false;
  }
  uniformBlockBinding(shaderProgram[ShaderProgram::InstancedLightPass]);
  uniformBlockBinding(shaderProgram[ShaderProgram::InstancedLightPass], "InstanceBuffer", 1);
  uniformBlockBinding(shaderProgram[ShaderProgram::InstancedLightPass], "LightBuffer", 2);

  // Shader program for light point visualization
  shaderProgram[ShaderProgram::InstancedLightVis] = glCreateProgram();
  glAttachShader(shaderProgram[ShaderProgram::InstancedLightVis], vertexShader[VertexShader::Light]);
  glAttachShader(shaderProgram[ShaderProgram::InstancedLightVis], fragmentShader[FragmentShader::LightColor]);
  if (!ShaderCompiler::LinkProgram(shaderProgram[ShaderProgram::InstancedLightVis]))
  {
    cleanUp();
    return false;
  }
  uniformBlockBinding(shaderProgram[ShaderProgram::InstancedLightVis]);
  uniformBlockBinding(shaderProgram[ShaderProgram::InstancedLightVis], "InstanceBuffer", 1);
  uniformBlockBinding(shaderProgram[ShaderProgram::InstancedLightVis], "LightBuffer", 2);

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
