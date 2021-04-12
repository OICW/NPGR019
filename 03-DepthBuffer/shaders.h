/*
 * Source code for the NPGR019 lab practices. Copyright Martin Kahoun 2021.
 * Licensed under the zlib license, see LICENSE.txt in the root directory.
 */

#pragma once

#include <ShaderCompiler.h>

// Shader programs
namespace ShaderProgram
{
  enum
  {
    Default, DepthVisualization, NumShaderPrograms
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
    Default, ScreenQuad, NumVertexShaders
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
layout (location = 0) uniform mat4 worldToView;
layout (location = 1) uniform mat4 projection;
layout (location = 2) uniform mat4 modelToWorld;

// Vertex attribute block, i.e., input
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

// Vertex output
out vec3 vColor;
out vec4 vViewPos;

void main()
{
  vec4 viewPos = worldToView * modelToWorld * vec4(position.xyz, 1.0f);

  vColor = color;
  vViewPos = viewPos;
  gl_Position = projection * viewPos;
}
)",
// ----------------------------------------------------------------------------
// Fullscreen quad vertex shader
// ----------------------------------------------------------------------------
R"(
#version 330 core

// Fullscreen quad
vec3 position[6] = vec3[6](vec3(-1.0f, -1.0f, 0.0f),
                           vec3( 1.0f, -1.0f, 0.0f),
                           vec3( 1.0f,  1.0f, 0.0f),
                           vec3( 1.0f,  1.0f, 0.0f),
                           vec3(-1.0f,  1.0f, 0.0f),
                           vec3(-1.0f, -1.0f, 0.0f));

// Quad UV coordinates
out vec2 UV;

void main()
{
  UV = position[gl_VertexID].xy * 0.5f + 0.5f;
  gl_Position = vec4(position[gl_VertexID].xyz, 1.0f);
}
)", ""};

// ============================================================================

// Fragment shader types
namespace FragmentShader
{
  enum
  {
    Default, DepthVisualization, NumFragmentShaders
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

// Fragment shader inputs
in vec3 vColor;
in vec4 vViewPos;

// Fragment shader outputs
layout (location = 0) out vec4 color;
layout (location = 1) out float view_z;

void main()
{
  // Output color to the color buffer
  color = vec4(vColor, 1.0f);
  // Output view space distance to the fragment to the positions buffer
  view_z = vViewPos.z;

  // We could forgo passing in the vViewPos position and just use
  // gl_FragCoord.w - but we must take into consideration that it
  // contains (x/w, y/w, z/w, 1/w), so we must output the result as:
  //view_z = 1.0f / gl_FragCoord.w;
}
)",
// ----------------------------------------------------------------------------
// Depth buffer visualization fragment shader source
// ----------------------------------------------------------------------------
R"(
#version 330 core

// The following is not not needed since GLSL version #430
#extension GL_ARB_explicit_uniform_location : require
// The following is not not needed since GLSL version #420
#extension GL_ARB_shading_language_420pack : require

// Uniform blocks, i.e., constants
layout (location = 0) uniform vec4 WIDTH_HEIGHT_MSAA_MODE;
layout (location = 1) uniform vec2 NEAR_FAR;

// Color, view positions and depth buffer texture
layout (binding = 0) uniform sampler2DMS colorBuffer;
layout (binding = 1) uniform sampler2DMS viewPosBuffer;
layout (binding = 2) uniform sampler2DMS depthBuffer;

// Quad UV coordinates
in vec2 UV;

// Output
out vec4 color;

void main()
{
  // Calculate the texel coordinates
  ivec2 texCoord = ivec2(UV * WIDTH_HEIGHT_MSAA_MODE.xy);

  // We can also use gl_FragCoord.xy directly
  //ivec2 texCoord = ivec2(gl_FragCoord.xy);

  // For all MSAA samples
  vec3 finalColor = vec3(0.0f);
  for (int i = 0; i < WIDTH_HEIGHT_MSAA_MODE.z; ++i)
  {
    if (WIDTH_HEIGHT_MSAA_MODE.w == 1)
    {
      // Just visualize the color
      vec3 c = texelFetch(colorBuffer, texCoord, i).rgb;
      finalColor += c;
    }
    else if (WIDTH_HEIGHT_MSAA_MODE.w == 2)
    {
      // Visualize the depth
      float d = texelFetch(depthBuffer, texCoord, i).r;
      finalColor.rgb += d;
    }
    else if (WIDTH_HEIGHT_MSAA_MODE.w == 3)
    {
      // Visualize the linear Z, remap it to [0, 1] range
      float z_linear = (texelFetch(viewPosBuffer, texCoord, i).r + NEAR_FAR.x) / (NEAR_FAR.y - NEAR_FAR.x);
      finalColor.rgb += z_linear;
    }
    else if (WIDTH_HEIGHT_MSAA_MODE.w == 4)
    {
      // Sample depth and linearize it by reverting the projection matrix transformation
      float d = texelFetch(depthBuffer, texCoord, i).r;
      // For [0, 1] depth range
      //float z = (NEAR_FAR.x * NEAR_FAR.y) / (NEAR_FAR.y - d * (NEAR_FAR.y + NEAR_FAR.x));
      // For [-1, 1] depth range, omitting 2* term to forgo division by 2 further
      float z = (NEAR_FAR.x * NEAR_FAR.y) / (NEAR_FAR.x + NEAR_FAR.y - d * (NEAR_FAR.y + NEAR_FAR.x));

      // Visualize the difference between depth and linear Z, remap it to [0, 1] range
      float z_linear = texelFetch(viewPosBuffer, texCoord, i).r + NEAR_FAR.x;
      finalColor.rgb += abs(z_linear - z) / (NEAR_FAR.y - NEAR_FAR.x);
    }
    else
    {
      finalColor = vec3(1.0f, 0.0f, 1.0f);
    }
  }

  // Discard pixels that are at the far plane of the depth buffer
  if (WIDTH_HEIGHT_MSAA_MODE.w > 1 && all(greaterThanEqual(finalColor.rgb, WIDTH_HEIGHT_MSAA_MODE.zzz))) discard;

  // Calculate average color
  color = vec4(finalColor.rgb / float(WIDTH_HEIGHT_MSAA_MODE.z), 1.0f);
}
)", ""};
