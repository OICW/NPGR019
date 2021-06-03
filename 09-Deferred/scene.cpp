/*
 * Source code for the NPGR019 lab practices. Copyright Martin Kahoun 2021.
 * Licensed under the zlib license, see LICENSE.txt in the root directory.
 */

#include "scene.h"
#include "shaders.h"

#include <functional>
#include <vector>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

#include <MathSupport.h>

// Scaling factor for lights movement curve
static const glm::vec3 scale = glm::vec3(13.0f, 2.0f, 13.0f);
// Offset for lights movement curve
static const glm::vec3 offset = glm::vec3(0.0f, 3.0f, 0.0f);

// Lissajous curve position calculation based on the parameters
auto lissajous = [](const glm::vec4 &p, float t) -> glm::vec3
{
  return glm::vec3(sinf(p.x * t), cosf(p.y * t), sinf(p.z * t) * cosf(p.w * t));
};

// ----------------------------------------------------------------------------

Scene& Scene::GetInstance()
{
  static Scene scene;
  return scene;
}

Scene::Scene() : _textures(Textures::GetInstance())
{

}

Scene::~Scene()
{
  // Delete meshes
  delete _quad;
  _quad = nullptr;
  delete _cube;
  _cube = nullptr;
  delete _icosahedron;
  _icosahedron = nullptr;

  // Release the instancing buffer
  glDeleteBuffers(1, &_instancingBuffer);

  // Release the light buffer
  glDeleteBuffers(1, &_lightBuffer);

  // Release the generic VAO
  glDeleteVertexArrays(1, &_vao);

  // Release textures
  glDeleteTextures(LoadedTextures::NumTextures, _loadedTextures);
}

void Scene::Init(int numCubes, int numLights)
{
  // Check if already initialized and return
  if (_vao)
    return;

  _numCubes = numCubes;
  _numLights = numLights;

  // Prepare meshes
  _quad = Geometry::CreateQuadNormalTangentTex();
  _cube = Geometry::CreateCubeNormalTangentTex();
  _icosahedron = Geometry::CreateIcosahedron();

  // Create general use VAO
  glGenVertexArrays(1, &_vao);

  {
    // Generate the instancing buffer as Uniform Buffer Object
    glGenBuffers(1, &_instancingBuffer);
    glBindBuffer(GL_UNIFORM_BUFFER, _instancingBuffer);

    // Obtain UBO index and size from the instancing shader program
    GLuint uboIndex = glGetUniformBlockIndex(shaderProgram[ShaderProgram::InstancedGBuffer], "InstanceBuffer");
    GLint uboSize = 0;
    glGetActiveUniformBlockiv(shaderProgram[ShaderProgram::InstancedGBuffer], uboIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &uboSize);

    // Describe the buffer data - we're going to change this every frame
    glBufferData(GL_UNIFORM_BUFFER, uboSize, nullptr, GL_DYNAMIC_DRAW);

    // Unbind the GL_UNIFORM_BUFFER target for now
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
  }

  {
    // Generate the light buffer as Uniform Buffer Object
    glGenBuffers(1, &_lightBuffer);
    glBindBuffer(GL_UNIFORM_BUFFER, _lightBuffer);

    // Obtain UBO index and size from the
    GLuint uboIndex = glGetUniformBlockIndex(shaderProgram[ShaderProgram::InstancedLightPass], "LightBuffer");
    GLint uboSize = 0;
    glGetActiveUniformBlockiv(shaderProgram[ShaderProgram::InstancedLightPass], uboIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &uboSize);

    // Describe the buffer data - we're going to change this every frame
    glBufferData(GL_UNIFORM_BUFFER, uboSize, nullptr, GL_DYNAMIC_DRAW);

    // Unbind the GL_UNIFORM_BUFFER target for now
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
  }

  {
    // Generate the transform UBO handle
    glGenBuffers(1, &_transformBlockUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, _transformBlockUBO);

    // Obtain UBO index from the default shader program:
    // we're gonna bind this UBO for all shader programs and we're making
    // assumption that all of the UBO's used by our shader programs are
    // all the same size
    GLuint uboIndex = glGetUniformBlockIndex(shaderProgram[ShaderProgram::DefaultGBuffer], "TransformBlock");
    GLint uboSize = 0;
    glGetActiveUniformBlockiv(shaderProgram[ShaderProgram::DefaultGBuffer], uboIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &uboSize);

    // Describe the buffer data - we're going to change this every frame
    glBufferData(GL_UNIFORM_BUFFER, uboSize, nullptr, GL_DYNAMIC_DRAW);

    // Bind the memory for usage
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, _transformBlockUBO);

    // Unbind the GL_UNIFORM_BUFFER target for now
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
  }

  // --------------------------------------------------------------------------

  // Position the first cube half a meter above origin
  _cubePositions.reserve(_numCubes);
  _cubePositions.push_back(glm::vec3(0.0f, 0.5f, 0.0f));

  // Generate random positions for the rest of the cubes
  for (int i = 1; i < _numCubes; ++i)
  {
    float x = getRandom(-5.0f, 5.0f);
    float y = getRandom( 1.0f, 5.0f);
    float z = getRandom(-5.0f, 5.0f);

    _cubePositions.push_back(glm::vec3(x, y, z));
  }

  // --------------------------------------------------------------------------

  // Ambient intensity for the lights
  const float ambientIntentsity = 1e-3f;

  // Calculate radius based on the light intensity
  auto getLightRadius = [](float r, float g, float b) -> float
  {
    // This is really hard and visible cutoff
    const float cutoff = 0.1f;
    float luminousIntensity = getLuminousIntensity(glm::vec3(r, g, b));
    return sqrt(luminousIntensity / cutoff);
  };

  // Position & color of the first light
  _lights.reserve(_numLights);
  glm::vec4 p = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);

  float r = 50.0f;
  float g = 50.0f;
  float b = 50.0f;
  glm::vec4 c = glm::vec4(r, g, b, ambientIntentsity);

  float radius = getLightRadius(r, g, b);

  _lights.push_back({glm::vec3(-3.0f, 3.0f, 0.0f), c, p, radius});

  // Generate random positions for the rest of the lights
  for (int i = 1; i < _numLights; ++i)
  {
    float x = getRandom(-2.0f, 2.0f);
    float y = getRandom(-2.0f, 2.0f);
    float z = getRandom(-2.0f, 2.0f);
    float w = getRandom(-2.0f, 2.0f);
    p = glm::vec4(x, y, z, w);

    r = getRandom(0.0f, 25.0f);
    g = getRandom(0.0f, 25.0f);
    b = getRandom(0.0f, 25.0f);
    c = glm::vec4(r, g, b, ambientIntentsity);

    radius = getLightRadius(r, g, b);

    _lights.push_back({offset + lissajous(p, 0.0f) * scale, c, p, radius});
  }

  // --------------------------------------------------------------------------

  // Create texture samplers
  _textures.CreateSamplers();

  // Prepare textures
  _loadedTextures[LoadedTextures::White] = Textures::CreateSingleColorTexture(255, 255, 255);
  _loadedTextures[LoadedTextures::Grey] = Textures::CreateSingleColorTexture(127, 127, 127);
  _loadedTextures[LoadedTextures::Blue] = Textures::CreateSingleColorTexture(127, 127, 255);
  _loadedTextures[LoadedTextures::CheckerBoard] = Textures::CreateCheckerBoardTexture(256, 16);
  _loadedTextures[LoadedTextures::Diffuse] = Textures::LoadTexture("data/Terracotta_Tiles_002_Base_Color.jpg", true);
  _loadedTextures[LoadedTextures::Normal] = Textures::LoadTexture("data/Terracotta_Tiles_002_Normal.jpg", false);
  _loadedTextures[LoadedTextures::Specular] = Textures::LoadTexture("data/Terracotta_Tiles_002_Roughness.jpg", false);
  _loadedTextures[LoadedTextures::Occlusion] = Textures::LoadTexture("data/Terracotta_Tiles_002_ambientOcclusion.jpg", false);
}

void Scene::Update(float dt, const Camera &camera)
{
  // Animation timer
  static float t = 0.0f;

  glm::vec3 cameraPos = camera.GetViewToWorld()[3];

  _insideLights.clear();
  _outsideLights.clear();

  // Assigns light set based on camera position, i.e., camera inside light volume or outside
  auto assignLightSet = [this](const glm::vec3 cameraPosition, int lightIdx)
  {
    glm::vec3 d = _lights[lightIdx].position - cameraPosition;
    float rSqr = _lights[lightIdx].radius * _lights[lightIdx].radius;

    if (glm::dot(d, d) < rSqr)
    {
      _insideLights.push_back(lightIdx);
    }
    else
    {
      _outsideLights.push_back(lightIdx);
    }
  };

  // Treat the first light as a special case with offset
  _lights[0].position = glm::vec3(-3.0f, 2.0f, 0.0f) + lissajous(_lights[0].movement, t);
  assignLightSet(cameraPos, 0);

  // Update the rest of the lights
  for (int i = 1; i < _numLights; ++i)
  {
    _lights[i].position = offset + lissajous(_lights[i].movement, t) * scale;
    assignLightSet(cameraPos, i);
  }

  // Update the animation timer
  t += dt;
}

void Scene::BindTextures(const GLuint &diffuse, const GLuint &normal, const GLuint &specular, const GLuint &occlusion)
{
  // We want to bind textures and appropriate samplers
  glActiveTexture(GL_TEXTURE0 + 0);
  glBindTexture(GL_TEXTURE_2D, diffuse);
  glBindSampler(0, _textures.GetSampler(Sampler::Anisotropic));

  glActiveTexture(GL_TEXTURE0 + 1);
  glBindTexture(GL_TEXTURE_2D, normal);
  glBindSampler(1, _textures.GetSampler(Sampler::Anisotropic));

  glActiveTexture(GL_TEXTURE0 + 2);
  glBindTexture(GL_TEXTURE_2D, specular);
  glBindSampler(2, _textures.GetSampler(Sampler::Anisotropic));

  glActiveTexture(GL_TEXTURE0 + 3);
  glBindTexture(GL_TEXTURE_2D, occlusion);
  glBindSampler(3, _textures.GetSampler(Sampler::Anisotropic));
}

void Scene::UpdateInstanceData()
{
  // Create transformation matrix
  glm::mat4x4 transformation = glm::mat4x4(1.0f);
  // Instance data CPU side buffer
  static std::vector<InstanceData> instanceData(MAX_INSTANCES);

  // Cubes
  float angle = 20.0f;
  for (int i = 0; i < _numCubes; ++i)
  {
    // Fill the transformation matrix
    transformation = glm::translate(_cubePositions[i]);
    transformation *= glm::rotate(glm::radians(i * angle), glm::vec3(1.0f, 1.0f, 1.0f));

    instanceData[i].transformation = glm::transpose(transformation);
  }

  // Start working with instancing buffer
  glBindBuffer(GL_UNIFORM_BUFFER, _instancingBuffer);

  // Bind the instancing buffer to the index 1
  glBindBufferBase(GL_UNIFORM_BUFFER, 1, _instancingBuffer);

  // Update the buffer data using mapping
  void *ptr = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
  memcpy(ptr, &*instanceData.begin(), _numCubes * sizeof(InstanceData));
  glUnmapBuffer(GL_UNIFORM_BUFFER);

  // Unbind the uniform buffer target
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

int Scene::UpdateLightData(LightSet lightSet, bool visualization)
{
  // Instance and light data CPU side buffer
  static std::vector<InstanceData> instanceData(MAX_INSTANCES);
  static std::vector<LightData> lightData(MAX_INSTANCES);

  // Based on the selected light pass let as pick light using the appropriate way, i.e.,
  // I'm doing here some lambda expressions magic because I'm lazy and lamdas are cool :)
  int numLights = 0;
  std::function<const Light &(int)> getLight;
  switch (lightSet)
  {
    case LightSet::All:
      numLights = _numLights;
      getLight = [this](int idx) -> const Light &
      {
        return _lights[idx];
      };
      break;

    case LightSet::Inside:
      numLights = (int)_insideLights.size();
      getLight = [this](int idx) -> const Light &
      {
        return _lights[_insideLights[idx]];
      };
      break;

    case LightSet::Outside:
      numLights = (int)_outsideLights.size();
      getLight = [this](int idx) -> const Light &
      {
        return _lights[_outsideLights[idx]];
      };
      break;
  }

  // Attenuation for visualization purposes
  const float attenuation = visualization ? 0.05f : 1.0f;
  // Scale of the light volume
  float scale;
  // Transformation matrix of the light volume
  glm::mat4x4 transformation = glm::mat4x4(1.0f);

  // For all lights in this light set
  for (int i = 0; i < numLights; ++i)
  {
    // Fetch the light from the _lights array using appropriate lambda expression
    const Light &light = getLight(i);

    // Apply scaling based on light intensity
    if (visualization)
    {
      scale = 0.1f;
    }
    else
    {
      scale = light.radius;
    }

    // Fill the transformation matrix
    transformation = glm::translate(light.position);
    transformation *= glm::scale(glm::vec3(scale));

    instanceData[i].transformation = glm::transpose(transformation);

    lightData[i].position = glm::vec4(light.position, light.radius);
    lightData[i].color = glm::vec4(light.color * attenuation);
  }

  {
    // Start working with instancing buffer
    glBindBuffer(GL_UNIFORM_BUFFER, _instancingBuffer);

    // Bind the instancing buffer to the index 1
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, _instancingBuffer);

    // Update the buffer data using mapping
    void *ptr = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
    memcpy(ptr, &*instanceData.begin(), (numLights) * sizeof(InstanceData));
    glUnmapBuffer(GL_UNIFORM_BUFFER);

    // Unbind the uniform buffer target
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
  }

  {
    // Start working with instancing buffer
    glBindBuffer(GL_UNIFORM_BUFFER, _instancingBuffer);

    // Bind the instancing buffer to the index 2
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, _lightBuffer);

    // Update the buffer data using mapping
    void *ptr = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
    memcpy(ptr, &*lightData.begin(), (numLights) * sizeof(LightData));
    glUnmapBuffer(GL_UNIFORM_BUFFER);

    // Unbind the uniform buffer target
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
  }

  return numLights;
}

void Scene::UpdateTransformBlock(const Camera &camera)
{
  // Tell OpenGL we want to work with our transform block
  glBindBuffer(GL_UNIFORM_BUFFER, _transformBlockUBO);

  // Note: we should properly obtain block members size and offset via
  // glGetActiveUniformBlockiv() with GL_UNIFORM_SIZE, GL_UNIFORM_OFFSET,
  // I'm yoloing it here...

  // Update the world to view transformation matrix - transpose to 3 columns, 4 rows for storage in an uniform block:
  // per std140 layout column matrix CxR is stored as an array of C columns with R elements, i.e., 4x3 matrix would
  // waste space because it would require padding to vec4
  glm::mat3x4 worldToView = glm::transpose(camera.GetWorldToView());
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat3x4), static_cast<const void*>(&*glm::value_ptr(worldToView)));

  // Update the projection matrix
  glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat3x4), sizeof(glm::mat4x4), static_cast<const void*>(&*glm::value_ptr(camera.GetProjection())));

  // Unbind the GL_UNIFORM_BUFFER target for now
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Scene::DrawBackground()
{
  GLuint program = shaderProgram[ShaderProgram::DefaultGBuffer];

  // Bind the shader program and update its data
  glUseProgram(program);

  // Bind textures
  BindTextures(_loadedTextures[LoadedTextures::CheckerBoard], _loadedTextures[LoadedTextures::Blue], _loadedTextures[LoadedTextures::Grey], _loadedTextures[LoadedTextures::White]);

  // Bind the geometry
  glBindVertexArray(_quad->GetVAO());

  // Draw floor:
  glm::mat4x4 transformation = glm::scale(glm::vec3(30.0f, 1.0f, 30.0f));
  glm::mat4x3 passMatrix = transformation;
  glUniformMatrix4x3fv(0, 1, GL_FALSE, glm::value_ptr(passMatrix));
  glDrawElements(GL_TRIANGLES, _quad->GetIBOSize(), GL_UNSIGNED_INT, reinterpret_cast<void*>(0));

  // Draw Z axis wall
  transformation = glm::translate(glm::vec3(0.0f, 0.0f, 15.0f));
  transformation *= glm::rotate(-PI_HALF, glm::vec3(1.0f, 0.0f, 0.0f));
  transformation *= glm::scale(glm::vec3(30.0f, 1.0f, 30.0f));
  passMatrix = transformation;
  glUniformMatrix4x3fv(0, 1, GL_FALSE, glm::value_ptr(passMatrix));
  glDrawElements(GL_TRIANGLES, _quad->GetIBOSize(), GL_UNSIGNED_INT, reinterpret_cast<void*>(0));

  // Draw X axis wall
  transformation = glm::translate(glm::vec3(15.0f, 0.0f, 0.0f));
  transformation *= glm::rotate(PI_HALF, glm::vec3(0.0f, 0.0f, 1.0f));
  transformation *= glm::scale(glm::vec3(30.0f, 1.0f, 30.0f));
  passMatrix = transformation;
  glUniformMatrix4x3fv(0, 1, GL_FALSE, glm::value_ptr(passMatrix));
  glDrawElements(GL_TRIANGLES, _quad->GetIBOSize(), GL_UNSIGNED_INT, reinterpret_cast<void*>(0));
}

void Scene::DrawObjects()
{
  // Update the instancing buffer
  UpdateInstanceData();

  GLuint program = shaderProgram[ShaderProgram::InstancedGBuffer];

  // Bind the shader program and update its data
  glUseProgram(program);

  // Bind textures
   BindTextures(_loadedTextures[LoadedTextures::Diffuse], _loadedTextures[LoadedTextures::Normal], _loadedTextures[LoadedTextures::Specular], _loadedTextures[LoadedTextures::Occlusion]);

  // Draw cubes
  glBindVertexArray(_cube->GetVAO());
  glDrawElementsInstanced(GL_TRIANGLES, _cube->GetIBOSize(), GL_UNSIGNED_INT, reinterpret_cast<void*>(0), _numCubes);
}

void Scene::DrawLights(const Camera &camera)
{
  auto lightPass = [this](LightSet lightSet, bool visualize)
  {
    // Update the instancing and light buffer
    int numLights = UpdateLightData(lightSet, visualize);

    if (numLights > 0)
    {
      glBindVertexArray(_icosahedron->GetVAO());
      glDrawElementsInstanced(GL_TRIANGLES, _icosahedron->GetIBOSize(), GL_UNSIGNED_INT, reinterpret_cast<void*>(0), numLights);
    }
  };

  // Bind the shader program for instanced light passes
  GLuint program = shaderProgram[ShaderProgram::InstancedLightPass];
  glUseProgram(program);

  // Update the camera world space position
  GLint loc = glGetUniformLocation(program, "cameraPosWS");
  const glm::vec4 &cameraPos = camera.GetViewToWorld()[3];
  glUniform4fv(loc, 1, glm::value_ptr(cameraPos));

  // Update the camera clip planes
  loc = glGetUniformLocation(program, "NEAR_FAR");
  glUniform2f(loc, camera.GetNearClip(), camera.GetFarClip());

  // Draw light volumes where camera is inside as back faces w/o depth test
  glCullFace(GL_FRONT);
  glDisable(GL_DEPTH_TEST);
  lightPass(LightSet::Inside, false);

  // Draw light volumes where camera is outside as back faces w/ depth test
  glCullFace(GL_BACK);
  glEnable(GL_DEPTH_TEST);
  lightPass(LightSet::Outside, false);

  // --------------------------------------------------------------------------
  // Draw light points

  // Bind the shader program for light point visualization
  glUseProgram(shaderProgram[ShaderProgram::InstancedLightVis]);

  // Draw light volumes as small points for visualization purposes
  lightPass(LightSet::All, true);
}

void Scene::DrawAmbientPass()
{
  GLuint program = shaderProgram[ShaderProgram::AmbientLightPass];

  // Bind the shader program and update its data
  glUseProgram(program);

  // Set the global ambient light
  const float lightIntensity = 0.1f;
  glUniform3f(0, lightIntensity, lightIntensity, lightIntensity);

  // Draw fullscreen quad - textures already bound outside the scope
  glBindVertexArray(_vao);
  glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Scene::Draw(const Camera &camera, const RenderTargets &renderTargets)
{
  UpdateTransformBlock(camera);

  // Enable depth test, clamp, and write
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_DEPTH_CLAMP);
  glDepthFunc(GL_LEQUAL);
  glDepthMask(GL_TRUE);

  // Enable backface culling
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  // --------------------------------------------------------------------------

  // Bind the GBuffer
  glBindFramebuffer(GL_FRAMEBUFFER, renderTargets.gBufferFbo);

  // Clear the color and depth buffers
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Render the scene into the GBuffer only
  DrawBackground();
  DrawObjects();

  // We primed the depth buffer, no need to write to it anymore
  glDepthMask(GL_FALSE);

  // --------------------------------------------------------------------------

  // Bind the HDR framebuffer
  glBindFramebuffer(GL_FRAMEBUFFER, renderTargets.hdrFbo);

  // Clear the color buffer
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  // Enable additive alpha blending
  glEnable(GL_BLEND);
  glBlendEquation(GL_FUNC_ADD);
  glBlendFunc(GL_ONE, GL_ONE);

  // Bind the GBuffer textures
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, renderTargets.depthStencil);
  glBindSampler(0, 0);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, renderTargets.colorRT);
  glBindSampler(1, 0);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, renderTargets.normalRT);
  glBindSampler(2, 0);
  glActiveTexture(GL_TEXTURE3);
  glBindTexture(GL_TEXTURE_2D, renderTargets.materialRT);
  glBindSampler(3, 0);

  // Combine the GBuffer into the HDR buffer using ambient light
  DrawAmbientPass();

  // Draw all the lights in the scene using the GBuffer as input outputting to the HDR buffer
  DrawLights(camera);

  // Disable blending
  glDisable(GL_BLEND);
}
