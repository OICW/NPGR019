/*
 * Source code for the NPGR019 lab practices. Copyright Martin Kahoun 2021.
 * Licensed under the zlib license, see LICENSE.txt in the root directory.
 */

#include "scene.h"
#include "shaders.h"

#include <vector>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

#include <MathSupport.h>

// Scaling factor for lights movement curve
static const glm::vec3 scale = glm::vec3(35.0f, 25.0f, 60.0f);

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

Scene::Scene()
{

}

Scene::~Scene()
{
  // Delete meshes
  delete _tetrahedron;
  _tetrahedron = nullptr;

  // Release the instancing buffer
  glDeleteBuffers(2, _sbo);

  // Release the generic VAO
  glDeleteVertexArrays(1, &_vao);
}

void Scene::Init(unsigned int workGroupSize, unsigned int numWorkGroups)
{
  // Check if already initialized and return
  if (_vao)
    return;

  _workGroupSize = workGroupSize;
  _numWorkGroups = numWorkGroups;
  _flockSize = _workGroupSize * _numWorkGroups;

  // Prepare meshes
  _tetrahedron = Geometry::CreateTetrahedron();

  // --------------------------------------------------------------------------

  // Create general use VAO
  glGenVertexArrays(1, &_vao);

  // Generate the instancing buffers
  glGenBuffers(ShaderData::NumBuffers, _sbo);

  // Create both VAOs for use with flocking simulation
  for (int i = 0; i < ShaderData::NumBuffers; ++i)
  {
    // Create the instancing buffer, it will be used for drawing and also updated by the GPU
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _sbo[ShaderData::Flock0 + i]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_INSTANCES * sizeof(InstanceData), nullptr, GL_DYNAMIC_COPY);
  }

  // Initialize data for the first frame
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, _sbo[ShaderData::Flock0]);
  InstanceData* data = reinterpret_cast<InstanceData*>(glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, _flockSize * sizeof(InstanceData), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));

  for (unsigned int i = 0; i < _flockSize; ++i)
  {
    // Generate position
    float x = getRandom(-150.0f, 150.0f);
    float y = getRandom(-150.0f, 150.0f);
    float z = getRandom(-150.0f, 150.0f);
    data[i].transformation[3] = glm::vec4(x, y, z, 1.0f);

    // Generate velocity
    x = getRandom(-0.5f, 0.5f);
    y = getRandom(-0.5f, 0.5f);
    z = getRandom(-0.5f, 0.5f);
    data[i].velocity = glm::vec4(x, y, z, 1.0f);

    // Set the aside, up, and dir using orthonormalization with scene up
    glm::vec3 direction = glm::normalize(glm::vec3(x, y, z));
    data[i].transformation[0] = glm::vec4(glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), direction)), 0.0f);
    data[i].transformation[1] = glm::vec4(glm::normalize(glm::cross(direction, glm::vec3(data[i].transformation[0]))), 0.0f);
    data[i].transformation[2] = glm::vec4(direction, 0.0f);
  }

  // Unmap and unbind the buffer for now
  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

  // --------------------------------------------------------------------------

  // Ambient intensity for the lights
  const float ambientIntentsity = 1e-3f;

  // Position & color of the light
  glm::vec4 p = glm::vec4(0.34f, 0.29f, 0.12f, 0.5f);
  _light = {lissajous(p, 0.0f) * scale, glm::vec4(100.0f, 100.0f, 100.0f, ambientIntentsity), p};
}

void Scene::Update(float dt, bool moveLight, bool turbo)
{
  // Animation timer
  static float t = 0.0f;
  // Frame index
  static unsigned int frameIndex = 0;

  // Update the light position
  _light.position = lissajous(_light.movement, t) * scale;

  // Update the animation timer
  t += moveLight ? dt : 0.0f;

  // --------------------------------------------------------------------------

  // Bind the simulation compute shader and update the goal position
  glUseProgram(shaderProgram[ShaderProgram::Flocking]);
  GLint goalLoc = glGetUniformLocation(shaderProgram[ShaderProgram::Flocking], "goal_dt");
  glUniform4f(goalLoc, _light.position.x, _light.position.y, _light.position.z, turbo ? dt * 10.0f : dt);

  // Bind input/output buffers
  unsigned int _previousFrameData = frameIndex & 0x01;
  unsigned int _currentFrameData = _previousFrameData ^ 0x01;
  // We will read from this buffer
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _sbo[_previousFrameData]);
  // We will put the simulation results to this buffer
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _sbo[_currentFrameData]);

  // Perform the simulation step in the compute shader
  glDispatchCompute(_numWorkGroups, 1, 1);

  // Unbind the input/output buffers
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);

  // Advance frame counter
  ++frameIndex;
}

void Scene::UpdateProgramData(GLuint program, const Camera &camera, const glm::vec3 &lightPosition, const glm::vec4 &lightColor)
{
  // Update the transformation & projection matrices
  glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(camera.GetWorldToView()));
  glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(camera.GetProjection()));

  // Update the light position
  GLint lightLoc = glGetUniformLocation(program, "lightPosWS");
  glUniform4f(lightLoc, lightPosition.x, lightPosition.y, lightPosition.z, 1.0f);

  // Update the view position
  GLint viewPosLoc = glGetUniformLocation(program, "viewPosWS");
  glm::vec4 viewPos = camera.GetViewToWorld()[3];
  glUniform4fv(viewPosLoc, 1, glm::value_ptr(viewPos));

  // Update the light color, 4th component controls ambient light intensity
  GLint lightColorLoc = glGetUniformLocation(program, "lightColor");
  glUniform4f(lightColorLoc, lightColor.x, lightColor.y, lightColor.z, lightColor.w);
}

void Scene::DrawObjects(GLuint program, const Camera &camera, const glm::vec3 &lightPosition, const glm::vec4 &lightColor)
{
  // Bind the shader program and update its data
  glUseProgram(program);
  // Update the transformation & projection matrices
  UpdateProgramData(program, camera, lightPosition, lightColor);

  // Bind the instancing buffer to the index 0
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _sbo[_currentFrameData]);

  // Draw the flock
  glBindVertexArray(_tetrahedron->GetVAO());
  glDrawElementsInstanced(GL_TRIANGLES, _tetrahedron->GetIBOSize(), GL_UNSIGNED_INT, reinterpret_cast<void*>(0), _flockSize);

  // Unbind the instancing buffer
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);

  // --------------------------------------------------------------------------

  // Draw the light object
  glUseProgram(shaderProgram[ShaderProgram::PointRendering]);

  // Update the transformation & projection matrices and other data
  glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(camera.GetWorldToView()));
  glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(camera.GetProjection()));
  glUniform3fv(2, 1, glm::value_ptr(lightPosition));

  // Update the color
  GLint colorLoc = glGetUniformLocation(shaderProgram[ShaderProgram::PointRendering], "color");
  glUniform3fv(colorLoc, 1, glm::value_ptr(lightColor));

  glPointSize(10.0f);
  glBindVertexArray(_vao);
  glDrawArrays(GL_POINTS, 0, 1);
}

void Scene::Draw(const Camera &camera, const RenderMode &renderMode)
{
  // Enable/disable MSAA rendering
  if (renderMode.msaaLevel > 1)
    glEnable(GL_MULTISAMPLE);
  else
    glDisable(GL_MULTISAMPLE);

  // Enable depth test, clamp, and write
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_DEPTH_CLAMP);
  glDepthFunc(GL_LEQUAL);

  // Enable backface culling
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  // Enable/disable wireframe
  glPolygonMode(GL_FRONT_AND_BACK, renderMode.wireframe ? GL_LINE : GL_FILL);

  // Clear the color and depth buffer
  glClearColor(0.01f, 0.02f, 0.04f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Draw all scene objects
  DrawObjects(shaderProgram[ShaderProgram::Instancing], camera, _light.position, _light.color);
}
