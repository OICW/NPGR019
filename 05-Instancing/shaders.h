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

// The following is not not needed since GLSL version #420
#extension GL_ARB_shading_language_420pack : require

// Uniform blocks, i.e., constants
layout (std140, binding = 0) uniform TransformBlock
{
  // Transposed worldToView matrix - stored compactly as an array of 3 x vec4
  mat3x4 worldToView;
  mat4x4 projection;
};

// Model to world transformation separately
layout (location = 0) uniform mat4x3 modelToWorld;

// Vertex attribute block, i.e., input
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;

// Vertex output
out vec2 vTexCoord;

void main()
{
  vTexCoord = texCoord;

  vec4 worldPos = vec4(modelToWorld * vec4(position.xyz, 1.0f), 1.0f);

  // We must multiply from the left because of transposed worldToView
  vec4 viewPos = vec4(worldPos * worldToView, 1.0f);

  gl_Position = projection * viewPos;
}
)",
// ----------------------------------------------------------------------------
// Instancing vertex shader using instanced vertex params
// ----------------------------------------------------------------------------
R"(
#version 330 core

// The following is not not needed since GLSL version #430
#extension GL_ARB_explicit_uniform_location : require

// The following is not not needed since GLSL version #420
#extension GL_ARB_shading_language_420pack : require

// Uniform blocks, i.e., constants
layout (std140, binding = 0) uniform TransformBlock
{
  // Transposed worldToView matrix - stored compactly as an array of 3 x vec4
  mat3x4 worldToView;
  mat4x4 projection;
};

// Vertex attribute block, i.e., input
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;

// Compactly stored modelToWorld matrices as 3 x vec4 vertex attributes (location 2, 3, 4)
layout (location = 2) in mat3x4 modelToWorld;

// Vertex output
out vec2 vTexCoord;

void main()
{
  vTexCoord = texCoord;

  // We must multiply from the left because of transposed modelToWorld and worldToView
  vec4 worldPos = vec4(vec4(position.xyz, 1.0f) * modelToWorld, 1.0f);
  vec4 viewPos = vec4(worldPos * worldToView, 1.0f);

  gl_Position = projection * viewPos;
}
)",
// ----------------------------------------------------------------------------
// Instancing vertex shader using instancing buffer via uniform buffer objects
// ----------------------------------------------------------------------------
R"(
#version 330 core

// The following is not not needed since GLSL version #430
#extension GL_ARB_explicit_uniform_location : require

// The following is not not needed since GLSL version #420
#extension GL_ARB_shading_language_420pack : require

// Uniform blocks, i.e., constants
layout (std140, binding = 0) uniform TransformBlock
{
  // Transposed worldToView matrix - stored compactly as an array of 3 x vec4
  mat3x4 worldToView;
  mat4x4 projection;
};

// Vertex attribute block, i.e., input
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;

// Must match the structure on the CPU side
struct InstanceData
{
  // Transposed worldToView matrix - stored compactly as an array of 3 x vec4
  mat3x4 modelToWorld;
};

// Uniform buffer used for instances
layout (std140, binding = 1) uniform InstanceBuffer
{
  // We are limited to 4096 vec4 registers in total, hence the maximum number of instances
  // being 1024 meaning we could fit another vec4 worth of data
  InstanceData instanceBuffer[1024];
};

// Vertex output
out vec2 vTexCoord;

void main()
{
  vTexCoord = texCoord;

  // Retrieve the model to world matrix from the instance buffer
  mat3x4 modelToWorld = instanceBuffer[gl_InstanceID].modelToWorld;

  // We must multiply from the left because of transposed modelToWorld and worldToView
  vec4 worldPos = vec4(vec4(position.xyz, 1.0f) * modelToWorld, 1.0f);
  vec4 viewPos = vec4(worldPos * worldToView, 1.0f);

  gl_Position = projection * viewPos;
}
)",
// ----------------------------------------------------------------------------
// Instancing vertex shader using instancing buffer via SSBO
// ----------------------------------------------------------------------------
R"(
#version 460 core

// Uniform blocks, i.e., constants
layout (std140, binding = 0) uniform TransformBlock
{
  // Transposed worldToView matrix - stored compactly as an array of 3 x vec4
  mat3x4 worldToView;
  mat4x4 projection;
};

// Vertex attribute block, i.e., input
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;

// Must match the structure on the CPU side
struct InstanceData
{
  // Transposed worldToView matrix - stored compactly as an array of 3 x vec4
  mat3x4 modelToWorld;
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
  mat3x4 modelToWorld = instanceBuffer.data[gl_InstanceID].modelToWorld;

  // We must multiply from the left because of transposed modelToWorld and worldToView
  vec4 worldPos = vec4(vec4(position.xyz, 1.0f) * modelToWorld, 1.0f);
  vec4 viewPos = vec4(worldPos * worldToView, 1.0f);

  gl_Position = projection * viewPos;
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
