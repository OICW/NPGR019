/*
 * Source code for the NPGR019 lab practices. Copyright Martin Kahoun 2021.
 * Licensed under the zlib license, see LICENSE.txt in the root directory.
 */

#pragma once

#include <Camera.h>
#include <Geometry.h>
#include <Textures.h>

// Textures we'll be using
namespace LoadedTextures
{
  enum
  {
    White, Grey, Blue, CheckerBoard, Diffuse, Normal, Specular, Occlusion, NumTextures
  };
}

namespace DisplayMode
{
  enum
  {
    Default, Color, Depth, Normals, Specular, Occlusion
  };
}

// Render mode structure
struct RenderMode
{
  // Vsync on?
  bool vsync;
  // Display mode for presentation
  int displayMode;
};

struct RenderTargets
{
  // Framebuffer object for HDR rendering
  GLuint hdrFbo = 0;
  // Framebuffer object for GBuffer rendering
  GLuint gBufferFbo = 0;
  // Render target for HDR rendering
  GLuint hdrRT = 0;
  // Depth stencil target
  GLuint depthStencil = 0;
  // Diffuse color buffer
  GLuint colorRT = 0;
  // Normals buffer
  GLuint normalRT = 0;
  // Material buffer
  GLuint materialRT = 0;
};

// Very simple scene abstraction class
class Scene
{
public:
  // Maximum number of allowed instances - must match the instancing vertex shader!
  static const unsigned int MAX_INSTANCES = 1024;

  // Get and create instance for this singleton
  static Scene& GetInstance();
  // Initialize the test scene
  void Init(int numCubes, int numLights);
  // Updates positions
  void Update(float dt, const Camera &camera);
  // Draw the scene
  void Draw(const Camera &camera, const RenderTargets &renderTargets);
  // Return the generic VAO for rendering
  GLuint GetGenericVAO() { return _vao; }

private:
  // GPU data for a single object instance
  struct InstanceData
  {
    // In this simple example just a transformation matrix, transposed for efficient storage
    glm::mat3x4 transformation;
  };

  // GPU data for a single light instance
  struct LightData
  {
    // Position of the light
    glm::vec4 position;
    // Color of the light
    glm::vec4 color;
  };

  // Structure describing light
  struct Light
  {
    // Position of the light
    glm::vec3 position;
    // Color and ambient intensity of the light
    glm::vec4 color;
    // Parameters for the light movement
    glm::vec4 movement;
    // Radius of the light based on luminous intensity and cutoff value
    float radius;
  };

  // Which light set to update and set to instance buffer
  enum class LightSet
  {
    All, Inside, Outside
  };

  // All is private, instance is created in GetInstance()
  Scene();
  ~Scene();
  // No copies allowed
  Scene(const Scene &);
  Scene & operator = (const Scene &);

  // Helper function for binding the appropriate textures
  void BindTextures(const GLuint &diffuse, const GLuint &normal, const GLuint &specular, const GLuint &occlusion);
  // Helper function for creating and updating the instance data
  void UpdateInstanceData();
  // Helper function for creating and updating light data
  int UpdateLightData(LightSet lightSet, bool visualization);
  // Helper method to update transformation uniform block
  void UpdateTransformBlock(const Camera &camera);
  // Draw the backdrop, floor and walls
  void DrawBackground();
  // Draw cubes
  void DrawObjects();
  // Draw lights
  void DrawLights(const Camera &camera);
  // Draw the ambient light fullscreen pass
  void DrawAmbientPass();

  // Textures helper instance
  Textures &_textures;
  // Loaded textures
  GLuint _loadedTextures[LoadedTextures::NumTextures] = {0};
  // Number of cubes in the scene
  int _numCubes = 10;
  // Cube positions
  std::vector<glm::vec3> _cubePositions;
  // Number of lights in the scene
  int _numLights;
  // All lights lights data
  std::vector<Light> _lights;
  // Indices of lights incident with camera
  std::vector<int> _insideLights;
  // Indices of lights well outside the camera
  std::vector<int> _outsideLights;
  // General use VAO
  GLuint _vao = 0;
  // Quad instance
  Mesh<Vertex_Pos_Nrm_Tgt_Tex> *_quad = nullptr;
  // Cube instance
  Mesh<Vertex_Pos_Nrm_Tgt_Tex> *_cube = nullptr;
  // Icosahedron instance for light rendering
  Mesh<Vertex_Pos> *_icosahedron = nullptr;
  // Instancing buffer handle
  GLuint _instancingBuffer = 0;
  // Light buffer handle
  GLuint _lightBuffer = 0;
  // Transformation matrices uniform buffer object
  GLuint _transformBlockUBO = 0;
};
