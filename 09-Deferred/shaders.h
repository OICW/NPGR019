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
    DefaultGBuffer, InstancedGBuffer, AmbientLightPass, InstancedLightPass, InstancedLightVis, Tonemapping, NumShaderPrograms
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
    Default, Instancing, Light, ScreenQuad, NumVertexShaders
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

// Camera position in world space coordinates
uniform vec4 cameraPosWS;
// Near/far clip planes for depth reconstruction
uniform vec2 NEAR_FAR;

// Vertex output
out VertexData
{
  // Represents a point in the far plane, hence no perspective division
  noperspective vec3 viewRayWS;
  // This is a light index and interpolation makes no sense
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

  // Output the WS view ray towards the far plane:
  // Take view direction from the worldToView inverse matrix (using transpose)
  vec3 viewDirWS = vec3(worldToView[2][0], worldToView[2][1], worldToView[2][2]);
  // Point along the viewDirWS in the far plane
  vec3 p = viewDirWS * NEAR_FAR.y;
  // Intersect ray from camera towards the WS vertex position with the far plane
  vec3 viewRayWS = worldPos.xyz - cameraPosWS.xyz;
  float t = dot(p, viewDirWS) / dot(viewRayWS, viewDirWS);
  // Pass the intersection to the fragment shader
  vOut.viewRayWS = viewRayWS * t;

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
    GBuffer, AmbientPass, LightPass, LightColor, Tonemapping, NumFragmentShaders
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
layout (location = 0) out vec3 oColor;
layout (location = 1) out vec2 oNormal;
layout (location = 2) out uvec3 oMaterial;

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

  // Just output the material properties into the GBuffer:
  // everything that we'll need to calculate lighting later on
  oColor = albedo;
  oNormal = normal.xz;

  // Pass information about normal orientation, just a single bit, 7 others free to use
  uint bitFlags = normal.y < 0.0f ? 1u : 0u;
  oMaterial = uvec3(specSample * 255.0f, occlusion * 255.0f, bitFlags);
}
)",
// ----------------------------------------------------------------------------
// Ambient light pass pixel shader
// ----------------------------------------------------------------------------
R"(
#version 330 core

// The following is not not needed since GLSL version #430
#extension GL_ARB_explicit_uniform_location : require

// The following is not not needed since GLSL version #420
#extension GL_ARB_shading_language_420pack : require

// GBuffer input textures
layout (binding = 0) uniform sampler2D Depth;
layout (binding = 1) uniform sampler2D Color;
layout (binding = 2) uniform sampler2D Normals;
layout (binding = 3) uniform usampler2D Material;

// Output color
out vec4 oColor;

// Global ambient light intensity and color
layout (location = 0) uniform vec3 ambientLight;

void main()
{
  // Get the fragment position
  ivec2 texel = ivec2(gl_FragCoord.xy);

  // Fetch the required GBuffer data
  vec3 albedo = texelFetch(Color, texel, 0).rgb;
  float occlusion = texelFetch(Material, texel, 0).g / 255.0f;

  // We're calculating here just the ambient light contribution to the scene,
  // but we could calculate directional light here as well
  vec3 ambient = occlusion * ambientLight.rgb;
  oColor = vec4(albedo * ambient, 1.0f);
}
)",
// ----------------------------------------------------------------------------
// Light pass pixel shader
// ----------------------------------------------------------------------------
R"(
#version 330 core

// The following is not not needed since GLSL version #420
#extension GL_ARB_shading_language_420pack : require

// All textures that can be sampled and displayed
layout (binding = 0) uniform sampler2D Depth;
layout (binding = 1) uniform sampler2D Color;
layout (binding = 2) uniform sampler2D Normals;
layout (binding = 3) uniform usampler2D Material;

// Must match the structure on the CPU side
struct LightData
{
  // Light position in world space
  vec4 positionWS;
  // Light color and intensity
  vec4 color;
};

// Uniform buffer used for lights
layout (std140) uniform LightBuffer
{
  // 1024 lights should be enough for everybody, must not exceed 4096 vec4 registers
  LightData lightBuffer[1024];
};

// Vertex inputs
in VertexData
{
  noperspective vec3 viewRayWS;
  flat int lightID;
} vIn;

// Camera position in world space coordinates
uniform vec4 cameraPosWS;
// Near/far clip planes for depth reconstruction
uniform vec2 NEAR_FAR;

// Output color
out vec4 oColor;

void main()
{
  ivec2 texel = ivec2(gl_FragCoord.xy);

  // Reconstruct the world space position using linearized sampled depth value
  const float near = NEAR_FAR.x;
  const float far = NEAR_FAR.y;
  float d = texelFetch(Depth, texel, 0).r;
  float z = (near * far) / (far + d * (near - far));
  // vIn.viewRayWS is at far plane, so just scale it down to the Z value
  vec3 posWS = cameraPosWS.xyz + vIn.viewRayWS * (z / far);

  // World space viewing direction
  vec3 viewDirWS = -normalize(vIn.viewRayWS);

  // Reconstruct the world space normal
  vec2 n = texelFetch(Normals, texel, 0).rg;
  uint bitFlags = texelFetch(Material, texel, 0).b;
  float y = (bitFlags == 1u ? -1.0f : 1.0f) * sqrt(max(1e-5, 1.0f - dot(n, n)));
  vec3 normalWS = vec3(n.r, y, n.g);

  // Fetch albedo and specularity
  vec3 albedo = texelFetch(Color, texel, 0).rgb;
  float specularity = texelFetch(Material, texel, 0).r / 255.0f;

  // Calculate the lighting direction and distance
  vec3 lightDirWS = lightBuffer[vIn.lightID].positionWS.xyz - posWS;
  float distSq = dot(lightDirWS, lightDirWS);
  float dist = sqrt(distSq);
  lightDirWS /= dist;

  // Need to make sure that distance function gets to 0 before leaving light volume
  float radius = lightBuffer[vIn.lightID].positionWS.w;
  float attenuation = 1.0f - smoothstep(0.66f * radius, 0.9f * radius, dist);

  // Calculate the halfway direction vector (cheaper approximation of
  // the reflected direction = reflect(-lightDirWS, normal)
  vec3 halfDirWS = normalize(viewDirWS + lightDirWS);

  // Calculate diffuse and specular coefficients
  float NdotL = max(0.0f, dot(normalWS, lightDirWS));
  float NdotH = max(0.0f, dot(normalWS, halfDirWS));

  // Calculate the Blinn-Phong model diffuse and specular terms
  vec3 lightColor = lightBuffer[vIn.lightID].color.rgb;
  vec3 diffuse = attenuation * NdotL * lightColor / distSq;
  vec3 specular = attenuation * specularity * lightColor * pow(NdotH, 64.0f) / distSq;

  // Calculate the final color
  vec3 finalColor = albedo * diffuse + specular;
  oColor = vec4(finalColor, 1.0f);
}
)",
// ----------------------------------------------------------------------------
// Light color pixel shader - for light visualization
// ----------------------------------------------------------------------------
R"(
#version 330 core

// Must match the structure on the CPU side
struct LightData
{
  // Light position in world space
  vec4 positionWS;
  // Light color and intensity
  vec4 color;
};

// Uniform buffer used for lights
layout (std140) uniform LightBuffer
{
  // 1024 lights should be enough for everybody, must not exceed 4096 vec4 registers
  LightData lightBuffer[1024];
};

// Vertex inputs
in VertexData
{
  noperspective vec3 viewRayWS;
  flat int lightID;
} vIn;

// Output color
out vec4 oColor;

void main()
{
  // Fetch the light color and output it
  oColor = vec4(lightBuffer[vIn.lightID].color.rgb, 1.0f);
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
layout (binding = 3) uniform usampler2D Material;
layout (binding = 4) uniform sampler2D HDR;

// Number of used MSAA samples
layout (location = 0) uniform vec3 NEAR_FAR_MODE;

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

  int MODE = int(NEAR_FAR_MODE.z);
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
    const float near = NEAR_FAR_MODE.x;
    const float far = NEAR_FAR_MODE.y;

    // Fetch depth and linearize it by reverting the projection matrix transformation
    float d = texelFetch(Depth, texel, 0).r;
    float z = (near * far) / (far + d * (near - far));

    // Remap it to [0, 1] range for display
    z = z / (far - near);

    finalColor = z.xxx;
  }
  else if (MODE == 3)
  {
    // Reconstruct world space normal and display it
    vec2 n = texelFetch(Normals, texel, 0).rg;
    uint bitFlags = texelFetch(Material, texel, 0).b;
    float y = (bitFlags == 1u ? -1.0f : 1.0f) * sqrt(max(1e-5, 1.0f - dot(n, n)));
    vec3 normal = vec3(n.r, y, n.g);
    finalColor = normal * 0.5f + 0.5f;
  }
  else if (MODE == 4)
  {
    // Fetch the material specularity value and display it
    finalColor = texelFetch(Material, texel, 0).rrr / 255.0f;
  }
  else if (MODE == 5)
  {
    // Fetch the material occlusion value and display it
    finalColor = texelFetch(Material, texel, 0).ggg / 255.0f;
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
