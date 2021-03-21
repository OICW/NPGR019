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
    Default, Instancing, PointRendering, Tonemapping, NumShaderPrograms
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
    Default, Instancing, Point, ScreenQuad, NumVertexShaders
  };
}

// Vertex shader sources
static const char* vsSource[] = {
// ----------------------------------------------------------------------------
// Default vertex shader
// ----------------------------------------------------------------------------
R"(
#version 460 core

// Uniform blocks, i.e., constants
layout (location = 0) uniform mat4 worldToView;
layout (location = 1) uniform mat4 projection;
layout (location = 2) uniform mat4 modelToWorld;

// Vertex attribute block, i.e., input
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 tangent;
layout (location = 3) in vec2 texCoord;

// Vertex output
out vec2 vTexCoord;
out vec3 vTangent;
out vec3 vBitangent;
out vec3 vNormal;
out vec4 vWorldPos;

void main()
{
  // Construct the normal transformation matrix
  mat3 normalTransform = mat3(transpose(inverse(modelToWorld)));

  // Create the tangent space matrix and pass it to the fragment shader
  vNormal = normalize(normalTransform * normal);
  vTangent = normalize(mat3(modelToWorld) * tangent);
  vBitangent = cross(vTangent, vNormal);

  // Transform vertex position
  vWorldPos = modelToWorld * vec4(position.xyz, 1.0f);
  gl_Position = projection * worldToView * vWorldPos;
  gl_Position = projection * worldToView * modelToWorld * vec4(position.xyz, 1.0f);

  // Pass texture coordinates to the fragment shader
  vTexCoord = texCoord.st;
}
)",
// ----------------------------------------------------------------------------
// Instancing vertex shader using instancing buffer
// ----------------------------------------------------------------------------
R"(
#version 460 core

// Uniform blocks, i.e., constants
layout (location = 0) uniform mat4 worldToView;
layout (location = 1) uniform mat4 projection;

// Vertex attribute block, i.e., input
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 tangent;
layout (location = 3) in vec2 texCoord;

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
out vec3 vTangent;
out vec3 vBitangent;
out vec3 vNormal;
out vec4 vWorldPos;

void main()
{
  // Retrieve the model to world matrix from the instance buffer
  mat4 modelToWorld = instanceBuffer.data[gl_InstanceID].modelToWorld;

  // Construct the normal transformation matrix
  mat3 normalTransform = mat3(transpose(inverse(modelToWorld)));

  // Create the tangent space matrix and pass it to the fragment shader
  vNormal = normalize(normalTransform * normal);
  vTangent = normalize(mat3(modelToWorld) * tangent);
  vBitangent = cross(vTangent, vNormal);

  // Transform vertex position
  vWorldPos = modelToWorld * vec4(position.xyz, 1.0f);
  gl_Position = projection * worldToView * vWorldPos;
  gl_Position = projection * worldToView * modelToWorld * vec4(position.xyz, 1.0f);

  // Pass texture coordinates to the fragment shader
  vTexCoord = texCoord.st;
}
)",
// ----------------------------------------------------------------------------
// Vertex shader for point rendering
// ----------------------------------------------------------------------------
R"(
#version 460 core

// Uniform blocks, i.e., constants
layout (location = 0) uniform mat4 worldToView;
layout (location = 1) uniform mat4 projection;
layout (location = 2) uniform vec3 position;

void main()
{
  gl_Position = projection * worldToView * vec4(position, 1.0f);
}
)",
// ----------------------------------------------------------------------------
// Fullscreen quad vertex shader
// ----------------------------------------------------------------------------
R"(
#version 460 core

// Fullscreen quad
vec3 position[] = {vec3(-1.0f, -1.0f, 0.0f),
                   vec3( 1.0f, -1.0f, 0.0f),
                   vec3( 1.0f,  1.0f, 0.0f),
                   vec3( 1.0f,  1.0f, 0.0f),
                   vec3(-1.0f,  1.0f, 0.0f),
                   vec3(-1.0f, -1.0f, 0.0f)};

// Quad UV coordinates
out vec2 UV;

void main()
{
  UV = position[gl_VertexID].xy * 0.5f + 0.5f;
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
    Default, SingleColor, Tonemapping, NumFragmentShaders
  };
}

// Fragment shader sources
static const char* fsSource[] = {
// ----------------------------------------------------------------------------
// Default fragment shader source
// ----------------------------------------------------------------------------
R"(
#version 460 core

// Texture sampler
layout (binding = 0) uniform sampler2D Diffuse;
layout (binding = 1) uniform sampler2D Normal;
layout (binding = 2) uniform sampler2D Specular;
layout (binding = 3) uniform sampler2D Occlusion;

// Light position/direction
uniform vec3 lightPosWS;
// View position in world space coordinates
uniform vec4 viewPosWS;

// Fragment shader inputs
in vec2 vTexCoord;
in vec3 vTangent;
in vec3 vBitangent;
in vec3 vNormal;
in vec4 vWorldPos;

// Fragment shader outputs
layout (location = 0) out vec4 color;

void main()
{
  // Normally you'd pass this as another uniform
  vec3 lightColor = vec3(100.0f, 100.0f, 100.0f);

  // Sample textures
  vec3 albedo = texture(Diffuse, vTexCoord.st).rgb;
  vec3 noSample = texture(Normal, vTexCoord.st).rgb;
  float specSample = texture(Specular, vTexCoord.st).r;
  float occlusion = texture(Occlusion, vTexCoord.st).r;

  // Calculate world-space normal
  mat3 STN = {vTangent, vBitangent, vNormal};
  vec3 normal = STN * (noSample * 2.0f - 1.0f);

  // Calculate the lighting direction and distance
  vec3 lightDir = lightPosWS.xyz - vWorldPos.xyz;
  float lengthSq = dot(lightDir, lightDir);
  float length = sqrt(lengthSq);
  lightDir /= length;

  // Calculate the view and reflection/halfway direction
  vec3 viewDir = normalize(viewPosWS.xyz - vWorldPos.xyz);
  // Cheaper approximation of reflected direction = reflect(-lightDir, normal)
  vec3 halfDir = normalize(viewDir + lightDir);

  // Calculate diffuse and specular coefficients
  float NdotL = max(0.0f, dot(normal, lightDir));
  float NdotH = max(0.0f, dot(normal, halfDir));

  // Calculate horizon fading factor
  float horizon = clamp(1.0f + dot(vNormal, lightDir), 0.0f, 1.0f);
  horizon *= horizon;
  horizon *= horizon;
  horizon *= horizon;
  horizon *= horizon;

  // Calculate the Phong model terms: ambient, diffuse, specular
  vec3 ambient = vec3(0.1f, 0.1f, 0.1f) * occlusion;
  vec3 diffuse = horizon * NdotL * lightColor / lengthSq;
  vec3 specular = horizon * specSample * lightColor * pow(NdotH, 64.0f) / lengthSq; // Defines shininess

  // Spotlight cone
  vec3 spotDir = normalize(lightPosWS.xyz);
  float theta = dot(lightDir, spotDir);
  float outer = 0.7f;
  float inner = 0.5f;
  float epsilon = outer - inner;
  float attenuation = clamp((theta - outer) / epsilon, 0.0f, 1.0f);
  diffuse *= attenuation;
  specular *= attenuation;

  // Calculate the final color
  vec3 finalColor = albedo * (ambient + diffuse) + specular;
  color = vec4(finalColor, 1.0f);
}
)",
// ----------------------------------------------------------------------------
// Single color pixel shader
// ----------------------------------------------------------------------------
R"(
#version 460 core

// Input color
layout (location = 3) uniform vec3 color;

// Output color
out vec4 oColor;

void main()
{
  oColor = vec4(color.rgb, 1.0f);
}
)",
// ----------------------------------------------------------------------------
// Tonemapping fragment shader source
// ----------------------------------------------------------------------------
R"(
#version 460 core

// Our HDR buffer texture
layout (binding = 0) uniform sampler2DMS HDR;

// Number of used MSAA samples
layout (location = 0) uniform float MSAA_LEVEL;

// Quad UV coordinates
in vec2 UV;

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
  // Query the size of the texture and calculate texel coordinates
  ivec2 texSize = textureSize(HDR);
  ivec2 texel = ivec2(UV * texSize);

  // Accumulate color for all MSAA samples
  vec3 finalColor = vec3(0.0f);
  for (int i = 0; i < int(MSAA_LEVEL); ++i)
  {
     // Fetch a single sample from a single texel (no interpolation)
     vec3 s = texelFetch(HDR, texel, i).rgb;
     finalColor += ApplyTonemapping(s);
  }

  color = vec4(finalColor.rgb / MSAA_LEVEL, 1.0f);
}
)",
""};
