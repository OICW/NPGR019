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

// Render mode structure
struct RenderMode
{
  // Vsync on?
  bool vsync;
  // Draw wireframe?
  bool wireframe;
  // Tonemapping on?
  bool tonemapping;
  // Used MSAA samples
  GLsizei msaaLevel;
};

// Very simple scene abstraction class
class Scene
{
public:
  // Data for a single object instance
  struct InstanceData
  {
    // Transformation matrix
    glm::mat4x4 transformation;
    // Velocity (vec4 to include padding)
    glm::vec4 velocity;
  };

  // Maximum number of allowed instances - SSBO can be up to 128 MB! - it'd be safer to ask driver, though
  static const unsigned int MAX_INSTANCES = 2 << 16;

  // Get and create instance for this singleton
  static Scene& GetInstance();
  // Initialize the test scene
  void Init(unsigned int workGroupSize, unsigned int numWorkGroups);
  // Updates positions
  void Update(float dt, bool moveLight, bool turbo);
  // Draw the scene
  void Draw(const Camera &camera, const RenderMode &renderMode);
  // Return the generic VAO for rendering
  GLuint GetGenericVAO() { return _vao; }

private:
  // Shader data indices for double buffering
  enum ShaderData
  {
    Flock0, Flock1, NumBuffers
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
  };

  // All is private, instance is created in GetInstance()
  Scene();
  ~Scene();
  // No copies allowed
  Scene(const Scene &);
  Scene & operator = (const Scene &);

  // Helper function for updating shader program data
  void UpdateProgramData(GLuint program, const Camera &camera, const glm::vec3 &lightPosition, const glm::vec4 &lightColor);
  // Draw cubes
  void DrawObjects(GLuint program, const Camera &camera, const glm::vec3 &lightPosition, const glm::vec4 &lightColor);

  // Size of the work group
  unsigned int _workGroupSize;
  // Size of the compute shader dispatch
  unsigned int _numWorkGroups;
  // Size of the whole flock
  unsigned int _flockSize;
  // Single Storage Buffer for all models used for instance data
  GLuint _sbo[ShaderData::NumBuffers];
  // Index of the SSBO to be read from
  unsigned int _previousFrameData;
  // Index of the SSBO to be written to and rendered from
  unsigned int _currentFrameData;
  // The single light object
  Light _light;
  // General use VAO
  GLuint _vao = 0;
  // Tetrahedron instance
  Mesh<Vertex_Pos_Nrm> *_tetrahedron = nullptr;
};
