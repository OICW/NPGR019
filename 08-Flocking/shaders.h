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
    Instancing, Flocking, PointRendering, Tonemapping, NumShaderPrograms
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
    Instancing, Point, ScreenQuad, NumVertexShaders
  };
}

// Vertex shader sources
static const char* vsSource[] = {
// ----------------------------------------------------------------------------
// Instancing vertex shader
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

// Structure holding per instance data
struct InstanceData
{
  // Model to world transformation
  mat4 modelToWorld;
  // Velocity - unused in the vertex shader
  vec4 velocity;
};

// Storage buffer used for instances using interface block syntax
layout (binding = 0) buffer InstanceBuffer
{
  // Only one variable length array allowed inside the storage buffer block
  InstanceData data[];
} instanceBuffer;

// Vertex output
out VertexData
{
  vec4 WorldPos;
  vec3 Normal;
  vec3 Color;
} v;

// From csflocking, OpenGL: SuperBible, 6th edition
vec3 generateColor(float f)
{
  float r = sin(f * 6.2831853f);
  float g = sin((f + 0.3333f) * 6.2831853f);
  float b = sin((f + 0.6666f) * 6.2831853f);

  return vec3(r, g, b) * 0.25f + vec3(0.75f);
}

void main()
{
  // Retrieve the model to world matrix from the instance buffer
  mat4 modelToWorld = instanceBuffer.data[gl_InstanceID].modelToWorld;

  // Construct the normal transformation matrix and transform normal
  mat3 normalTransform = mat3(transpose(inverse(modelToWorld)));
  v.Normal = normalize(normalTransform * normal);

  // Transform vertex position
  v.WorldPos = modelToWorld * vec4(position.xyz, 1.0f);
  gl_Position = projection * worldToView * v.WorldPos;

  // Generate color based on instance ID
  vec3 color = generateColor(fract(float(gl_InstanceID) / 1237.0f));
  v.Color = mix(color * 0.2f, color, smoothstep(0.0f, 0.8f, abs(normal.z)));
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
    Default, SingleColor, Null, Tonemapping, NumFragmentShaders
  };
}

// Fragment shader sources
static const char* fsSource[] = {
// ----------------------------------------------------------------------------
// Default fragment shader source
// ----------------------------------------------------------------------------
R"(
#version 460 core

// Light position/direction
uniform vec4 lightPosWS;
// View position in world space coordinates
uniform vec4 viewPosWS;
// Light color
uniform vec4 lightColor;

// Vertex input
in VertexData
{
  vec4 WorldPos;
  vec3 Normal;
  vec3 Color;
} v;

// Fragment shader outputs
layout (location = 0) out vec4 color;

void main()
{
  // Shortcut variables for ambient/diffuse light component intensity modulation
  const float ambientIntensity = lightColor.a;

  // Vertex parameters
  vec3 albedo = v.Color;
  vec3 normal = normalize(v.Normal);

  // Calculate the lighting direction and distance
  vec3 lightDir = lightPosWS.xyz - v.WorldPos.xyz;
  float lengthSq = dot(lightDir, lightDir);
  float length = sqrt(lengthSq);
  lightDir /= length;

  // Calculate the view and reflection/halfway direction
  vec3 viewDir = normalize(viewPosWS.xyz - v.WorldPos.xyz);
  // Cheaper approximation of reflected direction = reflect(-lightDir, normal)
  vec3 halfDir = normalize(viewDir + lightDir);

  // Calculate diffuse and specular coefficients
  float NdotL = max(0.0f, dot(normal, lightDir));
  float NdotH = max(0.0f, dot(normal, halfDir));

  // Calculate horizon fading factor
  float horizon = clamp(1.0f + dot(normal, lightDir), 0.0f, 1.0f);
  horizon *= horizon;
  horizon *= horizon;
  horizon *= horizon;
  horizon *= horizon;

  // Calculate the Phong model terms: ambient, diffuse, specular
  vec3 ambient = ambientIntensity * lightColor.rgb;
  vec3 diffuse = horizon * NdotL * lightColor.rgb / lengthSq;
  vec3 specular = horizon * 0.25f * lightColor.rgb * pow(NdotH, 32.0f) / lengthSq; // Defines shininess

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
// Null fragment shader shader for depth and stencil passes
// ----------------------------------------------------------------------------
R"(
#version 460 core

void main()
{
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

// ============================================================================

// Compute shader types
namespace ComputeShader
{
enum
{
  Flocking, NumComputeShaders
};
}

// Fragment shader sources
static const char* csSource[] = {
// ----------------------------------------------------------------------------
// Flocking compute shader source
// ----------------------------------------------------------------------------
R"(
#version 460 core

// ----------------------------------------------------------------------------
// Available system variables
// ----------------------------------------------------------------------------
// gl_NumWorkGroups
//     This variable contains the number of work groups passed to the dispatch function.
// gl_WorkGroupID
//     This is the current work group for this shader invocation. Each of the XYZ components will be on the half-open range [0, gl_NumWorkGroups.XYZ).
// gl_LocalInvocationID
//     This is the current invocation of the shader within the work group. Each of the XYZ components will be on the half-open range [0, gl_WorkGroupSize.XYZ).
// gl_GlobalInvocationID =  gl_WorkGroupID * gl_WorkGroupSize + gl_LocalInvocationID;
//     This value uniquely identifies this particular invocation of the compute shader among all invocations of this compute dispatch call.
// gl_LocalInvocationIndex
//    This is a 1D version of gl_LocalInvocationID. It identifies this invocation's index within the work group.
//    gl_LocalInvocationIndex =
//          gl_LocalInvocationID.z * gl_WorkGroupSize.x * gl_WorkGroupSize.y +
//          gl_LocalInvocationID.y * gl_WorkGroupSize.x +
//          gl_LocalInvocationID.x;
// ----------------------------------------------------------------------------

// Local work group size, i.e., how many invocations per work group
layout (local_size_x = 256) in;

// How close can flock members get together (squared)
uniform float closestDistanceSq = 50.0;
// Maximum allowed speed
uniform float maxSpeed = 10.0f;
// Weight rules
uniform vec4 ruleWeights = vec4(0.18f, 0.05f, 0.17f, 0.02f);
// Goal position which the flock will chase, timestep packed in the last component
uniform vec4 goal_dt;

// Structured buffer record
struct FlockMember
{
  // Model to world transformation matrix
  mat4 transformation;
  // Velocity
  vec4 velocity;
};

// Input structured buffer - previous frame state
layout (binding = 0) readonly buffer FlockIn
{
  FlockMember member[];
} inputData;

// Output structured buffer - current frame state (will be rendered)
layout (binding = 1) buffer FlockOut
{
  FlockMember member[];
} outputData;

// Workgroup shared storage (faster access than global memory, e.g., FlockIn buffer)
shared FlockMember membersCache[gl_WorkGroupSize.x];

// Rule #1: do not collide with others
vec3 collisionAvoidance(vec3 myPosition, vec3 myVelocity, vec3 otherPosition, vec3 otherVelocity)
{
  // Pull away when too close
  vec3 d = myPosition - otherPosition;
  if (dot(d, d) < closestDistanceSq)
    return d;

  return vec3(0.0f);
}

// Rule #2: fly in the general direction as others
vec3 followOthers(vec3 myPosition, vec3 myVelocity, vec3 otherPosition, vec3 otherVelocity)
{
  const float epsilonSq = 10.0f;
  vec3 d = otherPosition - myPosition;
  vec3 dv = otherVelocity - myVelocity;

  // Take acceleration as velocity difference modulated by distance, prevent it from getting too large
  return dv / (dot(d, d) + epsilonSq);
}

// For each member of the flock, we need to check all others, instead of reading
// straight from the input buffer, we'll cache the data in WorkGroupSize chunks
// to a shared local workgroup memory
void main()
{
  // Fetch our data from global memory
  FlockMember me = inputData.member[gl_GlobalInvocationID.x];

  // Our acceleration
  vec3 acceleration = vec3(0.0f);
  // Flock center
  vec3 flockCenter = vec3(0.0f);

  // Iterate over all work groups
  for (uint groupId = 0; groupId < gl_NumWorkGroups.x; ++groupId)
  {
    // Fetch one data member from global memory and put it in shared local cache
    membersCache[gl_LocalInvocationID.x] = inputData.member[groupId * gl_WorkGroupSize.x + gl_LocalInvocationID.x];

    // Wait until the whole work group fetched the data
    memoryBarrierShared();
    barrier();

    // Iterate over the local cache to apply first two flocking rules
    for (uint localId = 0; localId < gl_WorkGroupSize.x; ++localId)
    {
      FlockMember other = membersCache[localId];
      flockCenter += other.transformation[3].xyz;

      // Make sure we discard ourselves
      if (groupId * gl_WorkGroupSize.x + localId != gl_GlobalInvocationID.x)
      {
        acceleration += collisionAvoidance(me.transformation[3].xyz, me.velocity.xyz, other.transformation[3].xyz, other.velocity.xyz) * ruleWeights.x;
        acceleration += followOthers(me.transformation[3].xyz, me.velocity.xyz, other.transformation[3].xyz, other.velocity.xyz) * ruleWeights.y;
      }
    }

    // Catch up with the other threads in the work group
    barrier();
  }

  // Finish the update: Rule #3: follow common goal, Rule #4: try to reach flock center
  flockCenter /= float(gl_NumWorkGroups.x * gl_WorkGroupSize.x);
  acceleration += normalize(goal_dt.xyz - me.transformation[3].xyz) * ruleWeights.z;
  acceleration += normalize(flockCenter - me.transformation[3].xyz) * ruleWeights.w;

  // Update the new position and velocity
  vec3 position = me.transformation[3].xyz + me.velocity.xyz * goal_dt.w;
  vec3 velocity = me.velocity.xyz + acceleration * goal_dt.w;
  float speed = length(velocity);
  vec3 direction = velocity / speed;
  if (speed > maxSpeed)
  {
    velocity = direction * maxSpeed;
  }

  // Prepare the output data
  FlockMember newMe;
  newMe.velocity = vec4(velocity, 1.0f);

  // Update the transformation matrix (aside, up, direction, position)
  newMe.transformation[0] = vec4(normalize(cross(me.transformation[1].xyz, direction)), 0.0f);
  newMe.transformation[1] = vec4(normalize(cross(direction, newMe.transformation[0].xyz)), 0.0f);
  newMe.transformation[2] = vec4(direction, 0.0f);
  newMe.transformation[3] = vec4(position, 1.0f);

  // Write out to the output buffer
  outputData.member[gl_GlobalInvocationID.x] = newMe;
}
)",
""
};