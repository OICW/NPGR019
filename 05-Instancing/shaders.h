/*
 * Source code for the NPGR019 lab practices. Copyright Martin Kahoun 2021.
 * Licensed under the zlib license, see LICENSE.txt in the root directory.
 */

#pragma once

#include <ShaderCompiler.h>

// Uses instancing via SSBO buffer, requires OpenGL 4.3 and higher
#define _ALLOW_SSBO_INSTANCING 0

// Shader programs
namespace ShaderProgram
{
  enum
  {
    Default, VertexParamInstancing, InstancingUniformBlock, InstancingBuffer, NumShaderPrograms
  };
}

// Shader programs handle
extern GLuint shaderProgram[ShaderProgram::NumShaderPrograms];

// Helper function for creating and compiling the shaders
bool compileShaders();

// ============================================================================

// Vertex shader types
namespace VertexShader
{
  enum
  {
    Default, VertexParamInstancing, InstancingUniformBlock, InstancingBuffer, NumVertexShaders
  };
}

// Vertex shader sources
static const char* vsSource[] = {
// ----------------------------------------------------------------------------
// Default vertex shader
// ----------------------------------------------------------------------------
R"(
#version 330 core

// The following is not not needed since GLSL version #430
#extension GL_ARB_explicit_uniform_location : require

// Uniform blocks, i.e., constants
layout (std140) uniform TransformBlock
{
  mat4 worldToView;
  mat4 projection;
};

// Model to world transformation separately
layout (location = 0) uniform mat4 modelToWorld;

// Vertex attribute block, i.e., input
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;

// Vertex output
out vec2 vTexCoord;

void main()
{
  vTexCoord = texCoord;
  gl_Position = projection * worldToView * modelToWorld * vec4(position.xyz, 1.0f);
}
)",
// ----------------------------------------------------------------------------
// Instancing vertex shader using instanced vertex params
// ----------------------------------------------------------------------------
R"(
#version 330 core

// The following is not not needed since GLSL version #430
#extension GL_ARB_explicit_uniform_location : require

// Uniform blocks, i.e., constants
layout (std140) uniform TransformBlock
{
  mat4 worldToView;
  mat4 projection;
};

// Vertex attribute block, i.e., input
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in mat4 modelToWorld;

// Vertex output
out vec2 vTexCoord;

void main()
{
  vTexCoord = texCoord;
  gl_Position = projection * worldToView * modelToWorld * vec4(position.xyz, 1.0f);
}
)",
// ----------------------------------------------------------------------------
// Instancing vertex shader using instancing buffer via uniform block objects
// ----------------------------------------------------------------------------
R"(
#version 330 core

// The following is not not needed since GLSL version #430
#extension GL_ARB_explicit_uniform_location : require

// Uniform blocks, i.e., constants
layout (std140) uniform TransformBlock
{
  mat4 worldToView;
  mat4 projection;
};

// Vertex attribute block, i.e., input
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;

// Should match the structure on the CPU side
//struct InstanceData
//{
//  mat4 modelToWorld;
//};
//
//// Storage buffer used for instances using interface block syntax
//layout (binding = 0) buffer InstanceBuffer
//{
//  // Only one variable length array allowed inside the storage buffer block
//  InstanceData data[];
//} instanceBuffer;

// Vertex output
out vec2 vTexCoord;

void main()
{
  vTexCoord = texCoord;

  // Retrieve the model to world matrix from the instance buffer
  //mat4 modelToWorld = instanceBuffer.data[gl_InstanceID].modelToWorld;
  gl_Position = projection * worldToView /* modelToWorld*/ * vec4(position.xyz, 1.0f);
}
)",
// ----------------------------------------------------------------------------
// Instancing vertex shader using instancing buffer via SSDO
// ----------------------------------------------------------------------------
R"(
#version 460 core

// Uniform blocks, i.e., constants
layout (std140) uniform TransformBlock
{
  mat4 worldToView;
  mat4 projection;
};

// Vertex attribute block, i.e., input
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;

// Should match the structure on the CPU side
struct InstanceData
{
  mat4 modelToWorld;
};

// Storage buffer used for instances using interface block syntax
layout (binding = 0) buffer InstanceBuffer
{
  // Only one variable length array allowed inside the storage buffer block
  InstanceData data[];
} instanceBuffer;

// Vertex output
out vec2 vTexCoord;

void main()
{
  vTexCoord = texCoord;

  // Retrieve the model to world matrix from the instance buffer
  mat4 modelToWorld = instanceBuffer.data[gl_InstanceID].modelToWorld;
  gl_Position = projection * worldToView * modelToWorld * vec4(position.xyz, 1.0f);
}
)",
""};

// ============================================================================

// Fragment shader types
namespace FragmentShader
{
  enum
  {
    Default, NumFragmentShaders
  };
}

// Fragment shader sources
static const char* fsSource[] = {
// ----------------------------------------------------------------------------
// Default fragment shader source
// ----------------------------------------------------------------------------
R"(
#version 330 core

// The following is not not needed since GLSL version #420
#extension GL_ARB_shading_language_420pack : require

// Texture sampler
layout (binding = 0) uniform sampler2D diffuse;

// Fragment shader inputs
in vec2 vTexCoord;

// Fragment shader outputs
layout (location = 0) out vec4 color;

void main()
{
  // Output color to the color buffer
  vec3 texSample = texture(diffuse, vTexCoord.st).rgb;
  color = vec4(texSample, 1.0f);
}
)", ""};
