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
    DefaultGBuffer, InstancedGBuffer, AmbientLightPass, InstancedLightPass, PointRendering, Tonemapping, NumShaderPrograms
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
    Default, Instancing, Light, Point, ScreenQuad, NumVertexShaders
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
  // Transposed worldToView matrix - stored compactly as an array of 3 x vec4
  mat3x4 worldToView;
  mat4x4 projection;
};

// Model to world transformation separately
layout (location = 0) uniform mat4x3 modelToWorld;

// Vertex attribute block, i.e., input
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 tangent;
layout (location = 3) in vec2 texCoord;

// Vertex output
out VertexData
{
  vec2 texCoord;
  vec3 tangent;
  vec3 bitangent;
  vec3 normal;
  vec4 worldPos;
} vOut;

void main()
{
  // Pass texture coordinates to the fragment shader
  vOut.texCoord = texCoord.st;

  // Construct the normal transformation matrix
  mat3 normalTransform = transpose(inverse(mat3(modelToWorld)));

  // Create the tangent space matrix and pass it to the fragment shader
  vOut.normal = normalize(normalTransform * normal);
  vOut.tangent = normalize(mat3(modelToWorld) * tangent);
  vOut.bitangent = cross(vOut.tangent, vOut.normal);

  // Transform vertex position
  vOut.worldPos = vec4(modelToWorld * vec4(position.xyz, 1.0f), 1.0f);

  // We must multiply from the left because of transposed worldToView
  vec4 viewPos = vec4(vOut.worldPos * worldToView, 1.0f);

  gl_Position = projection * viewPos;
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
  // Transposed worldToView matrix - stored compactly as an array of 3 x vec4
  mat3x4 worldToView;
  mat4x4 projection;
};

// Vertex attribute block, i.e., input
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 tangent;
layout (location = 3) in vec2 texCoord;

// Must match the structure on the CPU side
struct InstanceData
{
  // Transposed worldToView matrix - stored compactly as an array of 3 x vec4
  mat3x4 modelToWorld;
};

// Uniform buffer used for instances
layout (std140) uniform InstanceBuffer
{
  // We are limited to 4096 vec4 registers in total, hence the maximum number of instances
  // being 1024 meaning we could fit another vec4 worth of data
  InstanceData instanceBuffer[1024];
};

// Vertex output
out VertexData
{
  vec2 texCoord;
  vec3 tangent;
  vec3 bitangent;
  vec3 normal;
  vec4 worldPos;
} vOut;

void main()
{
  // Pass texture coordinates to the fragment shader
  vOut.texCoord = texCoord.st;

  // Retrieve the model to world matrix from the instance buffer
  mat3x4 modelToWorld = instanceBuffer[gl_InstanceID].modelToWorld;

  // Construct the normal transformation matrix
  mat3 normalTransform = transpose(inverse(mat3(modelToWorld)));

  // Create the tangent space matrix and pass it to the fragment shader
  // Note: we must multiply from the left because of transposed modelToWorld
  vOut.normal = normalize(normal * normalTransform);
  vOut.tangent = normalize(tangent * mat3(modelToWorld));
  vOut.bitangent = cross(vOut.tangent, vOut.normal);

  // Transform vertex position, note we multiply from the left because of transposed modelToWorld
  vOut.worldPos = vec4(vec4(position.xyz, 1.0f) * modelToWorld, 1.0f);
  vec4 viewPos = vec4(vOut.worldPos * worldToView, 1.0f);

  gl_Position = projection * viewPos;
}
)",
// ----------------------------------------------------------------------------
// Instancing vertex shader for lights using instancing buffer via uniform block objects
// ----------------------------------------------------------------------------
R"(
#version 330 core

// The following is not not needed since GLSL version #430
#extension GL_ARB_explicit_uniform_location : require

// Uniform blocks, i.e., constants
layout (std140) uniform TransformBlock
{
  // Transposed worldToView matrix - stored compactly as an array of 3 x vec4
  mat3x4 worldToView;
  mat4x4 projection;
};

// Vertex attribute block, i.e., input
layout (location = 0) in vec3 position;

// Must match the structure on the CPU side
struct InstanceData
{
  // Transposed worldToView matrix - stored compactly as an array of 3 x vec4
  mat3x4 modelToWorld;
};

// Uniform buffer used for instances
layout (std140) uniform InstanceBuffer
{
  // We are limited to 4096 vec4 registers in total, hence the maximum number of instances
  // being 1024 meaning we could fit another vec4 worth of data
  InstanceData instanceBuffer[1024];
};

// Vertex output
out VertexData
{
  flat int lightID;
} vOut;

void main()
{
  // Pass in the instance ID to FS
  vOut.lightID = gl_InstanceID;

  // Retrieve the model to world matrix from the instance buffer
  mat3x4 modelToWorld = instanceBuffer[gl_InstanceID].modelToWorld;

  // Transform vertex position, note we multiply from the left because of transposed modelToWorld
  vec4 worldPos = vec4(vec4(position.xyz, 1.0f) * modelToWorld, 1.0f);
  vec4 viewPos = vec4(worldPos * worldToView, 1.0f);

  gl_Position = projection * viewPos;
}
)",
// ----------------------------------------------------------------------------
// Vertex shader for point rendering
// ----------------------------------------------------------------------------
R"(
#version 330 core

// Uniform blocks, i.e., constants
layout (std140) uniform TransformBlock
{
  // Transposed worldToView matrix - stored compactly as an array of 3 x vec4
  mat3x4 worldToView;
  mat4x4 projection;
};

uniform vec3 position;

void main()
{
  // We must multiply from the left because of transposed worldToView
  vec4 viewPos = vec4(vec4(position, 1.0f) * worldToView, 1.0f);
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

void main()
{
  gl_Position = vec4(position[gl_VertexID].xyz, 1.0f);
}
)",
""};

// ============================================================================

// Fragment shader types
namespace FragmentShader
{
  enum
  {
    GBuffer, AmbientPass, LightPass, SingleColor, Null, Tonemapping, NumFragmentShaders
  };
}

// Fragment shader sources
static const char* fsSource[] = {
// ----------------------------------------------------------------------------
// Fragment shader for GBuffer rendering
// ----------------------------------------------------------------------------
R"(
#version 330 core

// The following is not not needed since GLSL version #420
#extension GL_ARB_shading_language_420pack : require

// Texture sampler
layout (binding = 0) uniform sampler2D Diffuse;
layout (binding = 1) uniform sampler2D Normal;
layout (binding = 2) uniform sampler2D Specular;
layout (binding = 3) uniform sampler2D Occlusion;

// Light position/direction
uniform vec4 lightPosWS;
// View position in world space coordinates
uniform vec4 viewPosWS;
// Light color
uniform vec4 lightColor;

// Fragment shader inputs
in VertexData
{
  vec2 texCoord;
  vec3 tangent;
  vec3 bitangent;
  vec3 normal;
  vec4 worldPos;
} vIn;

// Fragment shader outputs
layout (location = 0) out vec4 color;

void main()
{
  // Sample textures
  vec3 albedo = texture(Diffuse, vIn.texCoord.st).rgb;
  vec3 noSample = texture(Normal, vIn.texCoord.st).rgb;
  float specSample = texture(Specular, vIn.texCoord.st).r;
  float occlusion = texture(Occlusion, vIn.texCoord.st).r;

  // Calculate world-space normal
  mat3 STN = {vIn.tangent, vIn.bitangent, vIn.normal};
  vec3 normal = STN * (noSample * 2.0f - 1.0f);

  // TODO
  color = vec4(albedo, 1.0f);
}
)",
// ----------------------------------------------------------------------------
// Ambient light pass pixel shader
// ----------------------------------------------------------------------------
R"(
#version 330 core

// Output color
out vec4 oColor;

void main()
{
  // TODO
  oColor = vec4(1, 0, 1, 1.0f);
}
)",
// ----------------------------------------------------------------------------
// Light pass pixel shader
// ----------------------------------------------------------------------------
R"(
#version 330 core

// Must match the structure on the CPU side
struct LightData
{
  // Light position in world space
  vec4 position;
  // Light color and intensity
  vec4 color;
};

// Uniform buffer used for lights
layout (std140) uniform LightBuffer
{
  // 1024 lights should be enough for everybody, must not exceed 4096 vec4 registers
  LightData instanceBuffer[1024];
};

// Vertex inputs
in VertexData
{
  flat int lightID;
} vIn;

// Output color
out vec4 oColor;

void main()
{
  // TODO
  oColor = vec4(1, 0, 1, 1.0f);
}
)",
// ----------------------------------------------------------------------------
// Single color pixel shader
// ----------------------------------------------------------------------------
R"(
#version 330 core

// Input color
uniform vec3 color;

// Output color
out vec4 oColor;

void main()
{
  oColor = vec4(color.rgb, 1.0f);
}
)",
// ----------------------------------------------------------------------------
// Null fragment shader shader for depth and stencil passes
// ----------------------------------------------------------------------------
R"(
#version 330 core

void main()
{
}
)",
// ----------------------------------------------------------------------------
// Fullscreen display fragment shader with tonemapping
// ----------------------------------------------------------------------------
R"(
#version 330 core

// The following is not not needed since GLSL version #430
#extension GL_ARB_explicit_uniform_location : require

// The following is not not needed since GLSL version #420
#extension GL_ARB_shading_language_420pack : require

// All textures that can be sampled and displayed
layout (binding = 0) uniform sampler2D Depth;
layout (binding = 1) uniform sampler2D Color;
layout (binding = 2) uniform sampler2D Normals;
layout (binding = 3) uniform sampler2D Material;
layout (binding = 4) uniform sampler2D HDR;

// Number of used MSAA samples
layout (location = 0) uniform int MODE;

// Output
out vec4 color;

vec3 ApplyTonemapping(vec3 hdr)
{
  // Reinhard global operator
  vec3 result = hdr / (hdr + vec3(1.0f));

  return result;
}

void main()
{
  // Get the fragment position
  ivec2 texel = ivec2(gl_FragCoord.xy);

  vec3 finalColor = vec3(0.0f);
  if (MODE == 0)
  {
     // Fetch an HDR texel and tonemap it
     vec3 hdr = texelFetch(HDR, texel, 0).rgb;
     finalColor += ApplyTonemapping(hdr);
  }
  else if (MODE == 1)
  {
    // Fetch the color and store it directly
    finalColor = texelFetch(Color, texel, 0).rgb;
  }
  else if (MODE == 2)
  {
    // Reconstruct world space normal and display it
    vec2 n = texelFetch(Normals, texel, 0).rg;
    float z = sqrt(max(1e-5, 1.0f - dot(n, n)));
    vec3 normal = vec3(n.x, n.y, n.z);
    finalColor = normal * 0.5f + 0.5f;
  }
  else if (MODE == 3)
  {
    // Fetch the material specularity value and display it
    finalColor = texelFetch(Material, texel, 0).rrr;
  }
  else if (MODE == 4)
  {
    // Fetch the material occlusion value and display it
    finalColor = texelFetch(Material, texel, 0).ggg;
  }
  else
  {
    // Display error color
    finalColor = vec3(1, 0, 1);
  }

  color = vec4(finalColor.rgb, 1.0f);
}
)",
""};
