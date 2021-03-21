/*
 * Source code for the NPGR019 lab practices. Copyright Martin Kahoun 2021.
 * Licensed under the zlib license, see LICENSE.txt in the root directory.
 */

#include <cstdio>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

#include <MathSupport.h>
#include <Camera.h>
#include <Geometry.h>
#include <Textures.h>

#include "shaders.h"

#define _ENABLE_OPENGL_DEBUG 0

// ----------------------------------------------------------------------------
// GLM optional parameters:
// GLM_FORCE_DEPTH_ZERO_TO_ONE - force projection depth mapping to [0, 1]
//                               must use glClipControl(), requires OpenGL 4.5
// GLM_FORCE_LEFT_HANDED       - use the left handed coordinate system
// GLM_FORCE_XYZW_ONLY         - simplify vector types and use x, y, z, w only
// ----------------------------------------------------------------------------

// Structure for holding window parameters
struct Window
{
  // Window default width
  static const int DefaultWidth = 800;
  // Window default height
  static const int DefaultHeight = 600;

  // Width in pixels
  int width;
  // Height in pixels
  int height;
  // Main window handle
  GLFWwindow *handle;
} mainWindow = {0};

// ----------------------------------------------------------------------------

// Near clip plane settings
float nearClipPlane = 0.1f;
// Far clip plane settings
float farClipPlane = 100.1f;
// Camera FOV
float fov = 45.0f;

// ----------------------------------------------------------------------------

// Mouse movement
struct MouseStatus
{
  // Current position
  double x, y;
  // Previous position
  double prevX, prevY;

  // Updates the status - called once per frame
  void Update(double &moveX, double &moveY)
  {
    moveX = x - prevX;
    prevX = x;
    moveY = y - prevY;
    prevY = y;
  }
} mouseStatus = {0.0};

// ----------------------------------------------------------------------------

// Camera movement speeds
static constexpr float CameraNormalSpeed = 5.0f;
static constexpr float CameraTurboSpeed = 50.0f;

// ----------------------------------------------------------------------------

// Maximum number of allowed instances - SSBO can be up to 128 MB! - it'd be safer to ask driver, though
static const unsigned int MAX_INSTANCES = 1024;
// Max buffer length
static const unsigned int MAX_TEXT_LENGTH = 256;
// MSAA samples
static const GLsizei MSAA_SAMPLES = 4;
// Used MSAA samples
GLsizei msaaLevel = MSAA_SAMPLES;

// Number of cubes in the scene
const int numCubes = 10;
// Cube position
std::vector<glm::vec3> cubePositions;


// Camera instance
Camera camera;
// Quad instance
Mesh<Vertex_Pos_Nrm_Tgt_Tex> *quad = nullptr;
// Cube instance
Mesh<Vertex_Pos_Nrm_Tgt_Tex> *cube = nullptr;
// Textures helper instance
Textures& textures(Textures::GetInstance());

// General use VAO
GLuint vao = 0;
// Our framebuffer object
GLuint fbo = 0;
// Our render target for rendering
GLuint renderTarget = 0;
// Our depth stencil for rendering
GLuint depthStencil = 0;

// Vsync on?
bool vsync = true;
// Depth test on?
bool depthTest = true;
// Draw wireframe?
bool wireframe = false;
// Tonemapping on?
bool tonemapping = true;
// Instancing buffer handle
GLuint instancingBuffer = 0;

// Data for a single object instance
struct InstanceData
{
  // In this simple example just a transformation matrix
  glm::mat4x4 transformation;
};

// ----------------------------------------------------------------------------

// Textures we'll be using
namespace LoadedTextures
{
  enum
  {
    White, Grey, Blue, CheckerBoard, Diffuse, Normal, Specular, Occlusion, NumTextures
  };
}
GLuint loadedTextures[LoadedTextures::NumTextures] = {0};

// ----------------------------------------------------------------------------

// Forward declaration for the framebuffer creation
void createFramebuffer(int width, int height, GLsizei MSAA);

// Callback for handling GLFW errors
void errorCallback(int error, const char* description)
{
  printf("GLFW Error %i: %s\n", error, description);
}

#if _ENABLE_OPENGL_DEBUG
void APIENTRY debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
{
  if (type == GL_DEBUG_TYPE_ERROR)
    printf("OpenGL error: %s\n", message);
}
#endif

// Callback for handling window resize events
void resizeCallback(GLFWwindow* window, int width, int height)
{
  mainWindow.width = width;
  mainWindow.height = height;
  glViewport(0, 0, width, height);
  camera.SetProjection(fov, (float)width / (float)height, nearClipPlane, farClipPlane);

  createFramebuffer(width, height, msaaLevel);
}

// Callback for handling mouse movement over the window - called when mouse movement is detected
void mouseMoveCallback(GLFWwindow* window, double x, double y)
{
  // Update the current position
  mouseStatus.x = x;
  mouseStatus.y = y;
}

// Keyboard callback for handling system switches
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  // Notify the window that user wants to exit the application
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  // Enable/disable MSAA - note that it still uses the MSAA buffer
  if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
  {
    if (msaaLevel > 1)
    {
      msaaLevel = 1;
    }
    else
    {
      msaaLevel = MSAA_SAMPLES;
    }

    createFramebuffer(mainWindow.width, mainWindow.height, msaaLevel);
  }

  // Enable/disable wireframe rendering
  if (key == GLFW_KEY_F2 && action == GLFW_PRESS)
  {
    wireframe = !wireframe;
  }

  // Enable/disable backface culling
  if (key == GLFW_KEY_F3 && action == GLFW_PRESS)
  {
    if (glIsEnabled(GL_CULL_FACE))
      glDisable(GL_CULL_FACE);
    else
      glEnable(GL_CULL_FACE);
  }

  // Enable/disable depth test
  if (key == GLFW_KEY_F4 && action == GLFW_PRESS)
  {
    depthTest = !depthTest;
  }

  // Enable/disable vsync
  if (key == GLFW_KEY_F5 && action == GLFW_PRESS)
  {
    vsync = !vsync;
    if (vsync)
      glfwSwapInterval(1);
    else
      glfwSwapInterval(0);
  }

  // Enable/disable vsync
  if (key == GLFW_KEY_F6 && action == GLFW_PRESS)
  {
    tonemapping = !tonemapping;
  }

  // Zoom in
  if (key == GLFW_KEY_KP_ADD || key == GLFW_KEY_EQUAL && action == GLFW_PRESS)
  {
    fov -= 1.0f;
    if (fov < 5.0f)
      fov = 5.0f;
  }

  // Zoom out
  if (key == GLFW_KEY_KP_SUBTRACT || key == GLFW_KEY_MINUS && action == GLFW_PRESS)
  {
    fov += 1.0f;
    if (fov >= 180.0f)
      fov = 179.0f;
  }

  if (key == GLFW_KEY_BACKSPACE && action == GLFW_PRESS)
  {
    fov = 45.0f;
  }

  // Set the camera projection
  camera.SetProjection(fov, (float)mainWindow.width / (float)mainWindow.height, nearClipPlane, farClipPlane);
}

// ----------------------------------------------------------------------------

// Helper method for loading textures
void loadTextures()
{
  // Create texture samplers
  textures.CreateSamplers();

  // Prepare textures
  loadedTextures[LoadedTextures::White] = Textures::CreateSingleColorTexture(255, 255, 255);
  loadedTextures[LoadedTextures::Grey] = Textures::CreateSingleColorTexture(127, 127, 127);
  loadedTextures[LoadedTextures::Blue] = Textures::CreateSingleColorTexture(127, 127, 255);
  loadedTextures[LoadedTextures::CheckerBoard] = Textures::CreateCheckerBoardTexture(256, 16);
  loadedTextures[LoadedTextures::Diffuse] = Textures::LoadTexture("data/Terracotta_Tiles_002_Base_Color.jpg", true);
  loadedTextures[LoadedTextures::Normal] = Textures::LoadTexture("data/Terracotta_Tiles_002_Normal.jpg", false);
  loadedTextures[LoadedTextures::Specular] = Textures::LoadTexture("data/Terracotta_Tiles_002_Roughness.jpg", false);
  loadedTextures[LoadedTextures::Occlusion] = Textures::LoadTexture("data/Terracotta_Tiles_002_ambientOcclusion.jpg", false);
}

// Helper method for creating scene geometry
void createGeometry()
{
  // Create general use VAO
  glGenVertexArrays(1, &vao);

  // Prepare meshes
  quad = Geometry::CreateQuadNormalTangentTex();
  cube = Geometry::CreateCubeNormalTangentTex();

  // Generate the instancing buffer but don't fill it with data
  glGenBuffers(1, &instancingBuffer);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, instancingBuffer);
  glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_INSTANCES * sizeof(InstanceData), nullptr, GL_DYNAMIC_DRAW);

  // Unbind the buffer for now
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

  // Position the first cube half a meter above origin
  cubePositions.reserve(numCubes);
  cubePositions.push_back(glm::vec3(0.0f, 0.5f, 0.0f));

  // Generate random positions for the rest of the cubes
  for (int i = 1; i <= numCubes; ++i)
  {
    float x = getRandom(-5.0f, 5.0f);
    float y = getRandom( 1.0f, 5.0f);
    float z = getRandom(-5.0f, 5.0f);

    cubePositions.push_back(glm::vec3(x, y, z));
  }
}

// Helper method for OpenGL initialization
bool initOpenGL()
{
  // Set the GLFW error callback
  glfwSetErrorCallback(errorCallback);

  // Initialize the GLFW library
  if (!glfwInit()) return false;

  // Request OpenGL 4.6 core profile upon window creation
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
#if _ENABLE_OPENGL_DEBUG
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // Create the window
  mainWindow.handle = glfwCreateWindow(Window::DefaultWidth, Window::DefaultHeight, "", nullptr, nullptr);
  if (mainWindow.handle == nullptr)
  {
    printf("Failed to create the GLFW window!");
    return false;
  }

  // Make the created window with OpenGL context current for this thread
  glfwMakeContextCurrent(mainWindow.handle);

  // Check that GLAD .dll loader and symbol imported is ready
  if (!gladLoadGL()) {
    printf("GLAD failed!\n");
    return false;
  }

#if _ENABLE_OPENGL_DEBUG
  // Enable error handling callback function - context must be created with DEBUG flags
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
  glDebugMessageCallback(debugCallback, nullptr);
  GLuint unusedIds = 0;
  glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, &unusedIds, true);
#endif

  // Enable vsync
  if (vsync)
    glfwSwapInterval(1);
  else
    glfwSwapInterval(0);

  // Enable backface culling
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  // Enable depth test
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

  // Enable depth remapping to [0, 1] interval
  glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);

  // Register a window resize callback
  glfwSetFramebufferSizeCallback(mainWindow.handle, resizeCallback);

  // Register keyboard callback
  glfwSetKeyCallback(mainWindow.handle, keyCallback);

  // Register mouse movement callback
  glfwSetCursorPosCallback(mainWindow.handle, mouseMoveCallback);

  // Set the OpenGL viewport and camera projection
  resizeCallback(mainWindow.handle, Window::DefaultWidth, Window::DefaultHeight);

  // Set the initial camera position and orientation
  camera.SetTransformation(glm::vec3(-3.0f, 3.0f, -5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

  return true;
}

// Helper function for creating the HDR framebuffer
void createFramebuffer(int width, int height, GLsizei MSAA)
{
  // Bind the default framebuffer
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // Generate the FBO if necessary
  if (!fbo)
  {
    glGenFramebuffers(1, &fbo);
  }

  // Bind it and recreate textures
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  // --------------------------------------------------------------------------
  // Render target texture:
  // --------------------------------------------------------------------------

  // Delete it if necessary
  if (glIsTexture(renderTarget))
  {
    glDeleteTextures(1, &renderTarget);
    renderTarget = 0;
  }

  // Create the texture name
  if (renderTarget == 0)
  {
    glGenTextures(1, &renderTarget);
  }

  // Bind and recreate the render target texture
  if (MSAA > 1)
  {
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, renderTarget);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, MSAA, GL_RGB16F, width, height, GL_TRUE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, renderTarget, 0);
  }
  else
  {
    glBindTexture(GL_TEXTURE_2D, renderTarget);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTarget, 0);
  }

  // --------------------------------------------------------------------------
  // Depth buffer texture:
  // --------------------------------------------------------------------------

  // Delete it if necessary
  if (glIsRenderbuffer(depthStencil))
  {
    glDeleteRenderbuffers(1, &depthStencil);
    depthStencil = 0;
  }

  // Create the depth-stencil name
  if (depthStencil == 0)
  {
    glGenRenderbuffers(1, &depthStencil);
  }

  // Bind and recreate the depth-stencil Render Buffer Object
  glBindRenderbuffer(GL_RENDERBUFFER, depthStencil);
  if (MSAA > 1)
  {
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, MSAA, GL_DEPTH_COMPONENT32F, width, height);
  }
  else
  {
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, width, height);
  }
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthStencil);

  // Set the list of draw buffers.
  GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, drawBuffers);

  // Check for completeness
  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status != GL_FRAMEBUFFER_COMPLETE)
  {
    printf("Failed to create framebuffer: 0x%04X\n", status);
  }

  // Bind back the window system provided framebuffer
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// Helper method for graceful shutdown
void shutDown()
{
  // Release shader programs
  for (int i = 0; i < ShaderProgram::NumShaderPrograms; ++i)
  {
    glDeleteProgram(shaderProgram[i]);
  }

  // Delete meshes
  delete quad;
  quad = nullptr;
  delete cube;
  cube = nullptr;

  // Release the instancing buffer
  glDeleteBuffers(1, &instancingBuffer);

  // Release the framebuffer
  glDeleteTextures(1, &renderTarget);
  glDeleteTextures(1, &depthStencil);
  glDeleteFramebuffers(1, &fbo);

  // Release the generic VAO
  glDeleteVertexArrays(1, &vao);

  // Release textures
  glDeleteTextures(LoadedTextures::NumTextures, loadedTextures);

  // Release the window
  glfwDestroyWindow(mainWindow.handle);

  // Close the GLFW library
  glfwTerminate();
}

// ----------------------------------------------------------------------------

// Helper method for handling input events
void processInput(float dt)
{
  // Camera movement - keyboard events
  int direction = (int)MovementDirections::None;
  if (glfwGetKey(mainWindow.handle, GLFW_KEY_W) == GLFW_PRESS)
    direction |= (int)MovementDirections::Forward;

  if (glfwGetKey(mainWindow.handle, GLFW_KEY_S) == GLFW_PRESS)
    direction |= (int)MovementDirections::Backward;

  if (glfwGetKey(mainWindow.handle, GLFW_KEY_A) == GLFW_PRESS)
    direction |= (int)MovementDirections::Left;

  if (glfwGetKey(mainWindow.handle, GLFW_KEY_D) == GLFW_PRESS)
    direction |= (int)MovementDirections::Right;

  if (glfwGetKey(mainWindow.handle, GLFW_KEY_R) == GLFW_PRESS)
    direction |= (int)MovementDirections::Up;

  if (glfwGetKey(mainWindow.handle, GLFW_KEY_F) == GLFW_PRESS)
    direction |= (int)MovementDirections::Down;

  // Camera speed
  if (glfwGetKey(mainWindow.handle, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    camera.SetMovementSpeed(CameraTurboSpeed);
  else
    camera.SetMovementSpeed(CameraNormalSpeed);

  // Update the mouse status
  double dx, dy;
  mouseStatus.Update(dx, dy);

  // Camera orientation - mouse movement
  glm::vec2 mouseMove(0.0f, 0.0f);
  if (glfwGetMouseButton(mainWindow.handle, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
  {
    mouseMove.x = (float)(dx);
    mouseMove.y = (float)(dy);
  }

  // Update the camera movement
  camera.Move((MovementDirections)direction, mouseMove, dt);

  // Reset camera state
  if (glfwGetKey(mainWindow.handle, GLFW_KEY_ENTER) == GLFW_PRESS)
  {
    camera.SetProjection(fov, (float)mainWindow.width / (float)mainWindow.height, nearClipPlane, farClipPlane);
    camera.SetTransformation(glm::vec3(-3.0f, 3.0f, -5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
  }
}

// Helper function for binding the appropriate textures
void bindTextures(const GLuint &diffuse, const GLuint &normal, const GLuint &specular, const GLuint &occlusion)
{
  // We want to bind textures and appropriate samplers
  glActiveTexture(GL_TEXTURE0 + 0);
  glBindTexture(GL_TEXTURE_2D, diffuse);
  glBindSampler(0, textures.GetSampler(Sampler::Anisotropic));

  glActiveTexture(GL_TEXTURE0 + 1);
  glBindTexture(GL_TEXTURE_2D, normal);
  glBindSampler(1, textures.GetSampler(Sampler::Anisotropic));

  glActiveTexture(GL_TEXTURE0 + 2);
  glBindTexture(GL_TEXTURE_2D, specular);
  glBindSampler(2, textures.GetSampler(Sampler::Anisotropic));

  glActiveTexture(GL_TEXTURE0 + 3);
  glBindTexture(GL_TEXTURE_2D, occlusion);
  glBindSampler(3, textures.GetSampler(Sampler::Anisotropic));
}

// Helper function for creating and updating the instance data
void updateInstanceData()
{
  // Create transformation matrix
  glm::mat4x4 transformation = glm::mat4x4(1.0f);
  // Instance data CPU side buffer
  static std::vector<InstanceData> instanceData(MAX_INSTANCES);

  // Cubes
  float angle = 20.0f;
  for (int i = 0; i < numCubes; ++i)
  {
    // Create unit matrix
    transformation = glm::translate(cubePositions[i]);
    transformation *= glm::rotate(glm::radians(i * angle), glm::vec3(1.0f, 1.0f, 1.0f));
    instanceData[i].transformation = transformation;
  }

  // Bind the instancing buffer to the index 0
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, instancingBuffer);

  // Update the buffer data using mapping
  void *ptr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
  memcpy(ptr, &*instanceData.begin(), (numCubes + 1) * sizeof(InstanceData));
  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

void updateProgramData(GLuint program, const glm::vec3 &lightPosition)
{
  // Update the transformation & projection matrices
  glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(camera.GetTransformation()));
  glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(camera.GetProjection()));

  // Update the light position
  GLint lightLoc = glGetUniformLocation(program, "lightPosWS");
  glUniform3f(lightLoc, lightPosition.x, lightPosition.y, lightPosition.z);

  // Update the view position
  GLint viewPosLoc = glGetUniformLocation(program, "viewPosWS");
  glm::vec4 viewPos = camera.GetInvTransformation()[3];
  glUniform4f(viewPosLoc, viewPos.x, viewPos.y, viewPos.z, viewPos.w);
}

void renderScene()
{
  // Update the scene
  updateInstanceData();

  // Bind the framebuffer
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  // Enable/disable depth test and write
  if (depthTest)
  {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);
  }
  else
  {
    glDisable(GL_DEPTH_TEST);
  }

  // Enable/disable MSAA rendering
  if (msaaLevel > 1)
    glEnable(GL_MULTISAMPLE);
  else
    glDisable(GL_MULTISAMPLE);

  // Enable/disable wireframe rendering
  if (wireframe)
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  else
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  // Clear the color and depth buffer
  glClearColor(0.1f, 0.2f, 0.4f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // --------------------------------------------------------------------------

  // Light position
  static glm::vec3 lightPosition(-3.0f, 3.0f, 0.0f);

  // Draw the scene floor:
  glUseProgram(shaderProgram[ShaderProgram::Default]);

  // Update the transformation & projection matrices
  {
    updateProgramData(shaderProgram[ShaderProgram::Default], lightPosition);

    glm::mat4x4 transformation = glm::scale(glm::vec3(30.0f, 1.0f, 30.0f));
    glUniformMatrix4fv(2, 1, GL_FALSE, glm::value_ptr(transformation));
  }

  bindTextures(loadedTextures[LoadedTextures::CheckerBoard], loadedTextures[LoadedTextures::Blue], loadedTextures[LoadedTextures::Grey], loadedTextures[LoadedTextures::White]);

  glBindVertexArray(quad->GetVAO());
  glDrawElements(GL_TRIANGLES, quad->GetIBOSize(), GL_UNSIGNED_INT, reinterpret_cast<void*>(0));

  // --------------------------------------------------------------------------

  // Draw cubes:
  glUseProgram(shaderProgram[ShaderProgram::Instancing]);

  // Update the transformation & projection matrices
  updateProgramData(shaderProgram[ShaderProgram::Instancing], lightPosition);

  bindTextures(loadedTextures[LoadedTextures::Diffuse], loadedTextures[LoadedTextures::Normal], loadedTextures[LoadedTextures::Specular], loadedTextures[LoadedTextures::Occlusion]);

  glBindVertexArray(cube->GetVAO());
  glDrawElementsInstanced(GL_TRIANGLES, cube->GetIBOSize(), GL_UNSIGNED_INT, reinterpret_cast<void*>(0), numCubes);

  // Unbind the instancing buffer (it was bound since the updateInstanceData() call
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);

  // --------------------------------------------------------------------------

  // Draw the light:
  glUseProgram(shaderProgram[ShaderProgram::PointRendering]);

  // Update the transformation & projection matrices and other data
  {
    glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(camera.GetTransformation()));
    glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(camera.GetProjection()));
    glUniform3fv(2, 1, glm::value_ptr(lightPosition));

    // Update the color
    GLint colorLoc = glGetUniformLocation(shaderProgram[ShaderProgram::PointRendering], "color");
    glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f);
  }

  glPointSize(10.0f);
  glBindVertexArray(vao);
  glDrawArrays(GL_POINTS, 0, 1);

  // --------------------------------------------------------------------------

  // Unbind the shader program and other resources
  glBindVertexArray(0);
  glUseProgram(0);

  if (tonemapping)
  {
    // Unbind the framebuffer and bind the window system provided FBO
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Solid fill always
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Disable multisampling and depth test
    glDisable(GL_MULTISAMPLE);
    glDisable(GL_DEPTH_TEST);

    // Clear the color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Tonemapping
    glUseProgram(shaderProgram[ShaderProgram::Tonemapping]);

    // Send in the required data
    glUniform1f(0, (float)msaaLevel);

    // Bind the HDR render target as texture
    GLenum target = (msaaLevel > 1) ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(target, renderTarget);
    glBindSampler(0, 0); // Very important!

    // Draw fullscreen quad
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Unbind the shader program and other resources
    glBindVertexArray(0);
    glUseProgram(0);
  }
  else
  {
    // Just copy the render target to the screen
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glDrawBuffer(GL_BACK);
    glBlitFramebuffer(0, 0, mainWindow.width, mainWindow.height, 0, 0, mainWindow.width, mainWindow.height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
  }
}

// Helper method for implementing the application main loop
void mainLoop()
{
  static double prevTime = 0.0;
  while (!glfwWindowShouldClose(mainWindow.handle))
  {
    // Calculate delta time
    double time = glfwGetTime();
    float dt = (float)(time - prevTime);
    prevTime = time;

    // Print it to the title bar
    static char title[MAX_TEXT_LENGTH];
    static char instacing[] = "[Instancing] ";
    snprintf(title, MAX_TEXT_LENGTH, "dt = %.2fms, FPS = %.1f", dt * 1000.0f, 1.0f / dt);
    glfwSetWindowTitle(mainWindow.handle, title);

    // Poll the events like keyboard, mouse, etc.
    glfwPollEvents();

    // Process keyboard input
    processInput(dt);

    // Render the scene
    renderScene();

    // Swap actual buffers on the GPU
    glfwSwapBuffers(mainWindow.handle);
  }
}

int main()
{
  // Initialize the OpenGL context and create a window
  if (!initOpenGL())
  {
    printf("Failed to initialize OpenGL!\n");
    shutDown();
    return -1;
  }

  // Compile shaders needed to run
  if (!compileShaders())
  {
    printf("Failed to compile shaders!\n");
    shutDown();
    return -1;
  }

  // Create the scene geometry
  createGeometry();

  // Load & create texture
  loadTextures();

  // Enter the application main loop
  mainLoop();

  // Release used resources and exit
  shutDown();
  return 0;
}
