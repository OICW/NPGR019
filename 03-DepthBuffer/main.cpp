/*
 * Source code for the NPGR019 lab practices. Copyright Martin Kahoun 2021.
 * Licensed under the zlib license, see LICENSE.txt in the root directory.
 */

#include <cstdio>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

#include <Camera.h>
#include <Geometry.h>

#include "shaders.h"

#define _DEPTH_PRECISION_TEST 0

// Set to 1 to create debugging context that reports errors, requires OpenGL 4.3!
#define _ENABLE_OPENGL_DEBUG 0

// ----------------------------------------------------------------------------
// GLM optional parameters:
// GLM_FORCE_LEFT_HANDED       - use the left handed coordinate system
// GLM_FORCE_XYZW_ONLY         - simplify vector types and use x, y, z, w only
// ----------------------------------------------------------------------------
// For remapping depth to [0, 1] interval use GLM option below with glClipControl
// glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE); // requires version >= 4.5
//
// GLM_FORCE_DEPTH_ZERO_TO_ONE - force projection depth mapping to [0, 1]
//                               must use glClipControl(), requires OpenGL 4.5
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

// Max buffer length
static const unsigned int MAX_TEXT_LENGTH = 256;
// MSAA samples
static const GLsizei MSAA_SAMPLES = 4;
// Used MSAA samples
GLsizei msaaLevel = MSAA_SAMPLES;
// Camera instance
Camera camera;
// Cube instance
Mesh<Vertex_Pos_Col> *cube = nullptr;
// Quad instance
Mesh<Vertex_Pos_Col> *quad = nullptr;

// Vsync on?
bool vsync = true;
// Depth test on?
bool depthTest = true;
// Visualize depth buffer?
bool visualizeDepth = false;
// Mode of visualization
int mode = 1;

// General use VAO
GLuint vao = 0;
// Our framebuffer object
GLuint fbo = 0;
// Our render target for rendering
GLuint renderTarget = 0;
// Our render target for view space pos
GLuint viewSpacePos = 0;
// Our depth stencil for rendering
GLuint depthStencil = 0;

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
    GLint polygonMode[2];
    glGetIntegerv(GL_POLYGON_MODE, polygonMode);
    if (polygonMode[0] == GL_FILL)
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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

  // Depth buffer visualization modes
  if (key == GLFW_KEY_F6 && action == GLFW_PRESS)
  {
    visualizeDepth = !visualizeDepth;
  }

  if (key == GLFW_KEY_1 && action == GLFW_PRESS)
  {
    mode = 1; // Color (default)
  }

  if (key == GLFW_KEY_2 && action == GLFW_PRESS)
  {
    mode = 2; // Depth buffer
  }

  if (key == GLFW_KEY_3 && action == GLFW_PRESS)
  {
    mode = 3; // Linear depth
  }

  if (key == GLFW_KEY_4 && action == GLFW_PRESS)
  {
    mode = 4; // Difference between depth buffer and linear depth
  }
}

// ----------------------------------------------------------------------------

// Helper method for creating scene geometry
void createGeometry()
{
  // Create general use VAO
  glGenVertexArrays(1, &vao);

  quad = Geometry::CreateQuadColor();
  cube = Geometry::CreateCubeColor();
}

// Helper method for OpenGL initialization
bool initOpenGL()
{
  // Set the GLFW error callback
  glfwSetErrorCallback(errorCallback);

  // Initialize the GLFW library
  if (!glfwInit()) return false;

  // Request OpenGL 3.3 core profile upon window creation
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_SAMPLES, 0);
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

  // Register a window resize callback
  glfwSetFramebufferSizeCallback(mainWindow.handle, resizeCallback);

  // Register keyboard callback
  glfwSetKeyCallback(mainWindow.handle, keyCallback);

  // Register mouse movement callback
  glfwSetCursorPosCallback(mainWindow.handle, mouseMoveCallback);

  // Set the OpenGL viewport and camera projection
  resizeCallback(mainWindow.handle, Window::DefaultWidth, Window::DefaultHeight);

  // Set the initial camera position and orientation
#if _DEPTH_PRECISION_TEST
  camera.SetTransformation(glm::vec3(-3.0f, 3.0f, -5.0f), glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 1.0f, 0.0f));
#else
  camera.SetTransformation(glm::vec3(-3.0f, 3.0f, -5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
#endif

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
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, MSAA, GL_RGBA8, width, height, GL_TRUE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, renderTarget, 0);
  }
  else
  {
    glBindTexture(GL_TEXTURE_2D, renderTarget);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTarget, 0);
  }

  // --------------------------------------------------------------------------
  // View space position texture:
  // --------------------------------------------------------------------------

  // Delete it if necessary
  if (glIsTexture(viewSpacePos))
  {
    glDeleteTextures(1, &viewSpacePos);
    viewSpacePos = 0;
  }

  // Create the texture name
  if (viewSpacePos == 0)
  {
    glGenTextures(1, &viewSpacePos);
  }

  // Bind and recreate the render target texture
  if (MSAA > 1)
  {
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, viewSpacePos);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, MSAA, GL_R32F, width, height, GL_TRUE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D_MULTISAMPLE, viewSpacePos, 0);
  }
  else
  {
    glBindTexture(GL_TEXTURE_2D, viewSpacePos);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, viewSpacePos, 0);
  }

  // --------------------------------------------------------------------------
  // Depth buffer texture:
  // --------------------------------------------------------------------------

  // Delete it if necessary
  if (glIsTexture(depthStencil))
  {
    glDeleteTextures(1, &depthStencil);
    depthStencil = 0;
  }

  // Create the depth-stencil name
  if (depthStencil == 0)
  {
    glGenTextures(1, &depthStencil);
  }

  // Bind and recreate the depth-stencil texture, if you don't intend to read the depth buffer,
  // you can create it as Render Buffer Object:
  //    glGenRenderbuffers(depthStencil);
  //    glBindRenderbuffer(GL_RENDERBUFFER, depthStencil);
  //    glRenderbufferStorageMultisample(GL_RENDERBUFFER, MSAA_SAMPLES, GL_DEPTH24_STENCIL8, windowWidth, windowHeight);
  //    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthStencil);

  // Internal format, determines precision, possible choices (depth only)
  // GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT32F
  GLint format = GL_DEPTH_COMPONENT32F;
  if (MSAA > 1)
  {
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, depthStencil);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, MSAA, format, width, height, GL_TRUE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, depthStencil, 0);
  }
  else
  {
    glBindTexture(GL_TEXTURE_2D, depthStencil);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_DEPTH_COMPONENT, GL_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthStencil, 0);
  }

  // Set the list of draw buffers.
  GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
  glDrawBuffers(2, drawBuffers);

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

  // Release the framebuffer
  glDeleteTextures(1, &renderTarget);
  glDeleteTextures(1, &viewSpacePos);
  glDeleteTextures(1, &depthStencil);
  glDeleteFramebuffers(1, &fbo);

  // Release the generic VAO
  glDeleteVertexArrays(1, &vao);

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
#if _DEPTH_PRECISION_TEST
    camera.SetTransformation(glm::vec3(-3.0f, 3.0f, -5.0f), glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 1.0f, 0.0f));
#else
    camera.SetTransformation(glm::vec3(-3.0f, 3.0f, -5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
#endif
  }
}

void renderScene()
{
  // Bind the framebuffer
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  // Enable/disable MSAA rendering
  if (msaaLevel > 1)
    glEnable(GL_MULTISAMPLE);
  else
    glDisable(GL_MULTISAMPLE);

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

  // --------------------------------------------------------------------------

  // Clear the color and depth buffer
  glClearColor(0.1f, 0.2f, 0.4f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Tell OpenGL we'd like to use the previously compiled shader program
  glUseProgram(shaderProgram[ShaderProgram::Default]);

  // Update the transformation & projection matrices
  glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(camera.GetWorldToView()));
  glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(camera.GetProjection()));

  // --------------------------------------------------------------------------

  // Create transformation matrix
  glm::mat4x4 transformation = glm::mat4x4(1.0f);
#if !_DEPTH_PRECISION_TEST
  transformation *= glm::scale(glm::vec3(30.0f, 1.0f, 30.0f));
  glUniformMatrix4fv(2, 1, GL_FALSE, glm::value_ptr(transformation));

  // Draw the quad
  glBindVertexArray(quad->GetVAO());
  glDrawElements(GL_TRIANGLES, quad->GetIBOSize(), GL_UNSIGNED_INT, reinterpret_cast<void*>(0));
#endif

  // Update transformation matrix for the cube
  transformation = glm::mat4x4(1.0f);
  transformation *= glm::translate(glm::vec3(0.0f, 0.5f, 0.0f));
  glUniformMatrix4fv(2, 1, GL_FALSE, glm::value_ptr(transformation));

  // Draw the cube
  glBindVertexArray(cube->GetVAO());
  glDrawElements(GL_TRIANGLES, cube->GetIBOSize(), GL_UNSIGNED_INT, reinterpret_cast<void*>(0));

#if _DEPTH_PRECISION_TEST
  for (int i = 0; i < 10; ++i)
  {
    transformation = glm::mat4x4(1.0f);
    transformation *= glm::translate(glm::vec3(0.0f, 0.5f, 5.0f + i * 5.0f));
    glUniformMatrix4fv(2, 1, GL_FALSE, glm::value_ptr(transformation));

    // Draw the cube
    glBindVertexArray(cube->GetVAO());
    glDrawElements(GL_TRIANGLES, cube->GetIBOSize(), GL_UNSIGNED_INT, reinterpret_cast<void*>(0));
  }
#endif

  // --------------------------------------------------------------------------

  // Unbind the shader program and other resources
  glBindVertexArray(0);
  glUseProgram(0);

  // --------------------------------------------------------------------------

  if (visualizeDepth)
  {
    // Solid fill always
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Unbind the framebuffer and bind the window system provided FBO
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Clear the color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Visualization shader program
    glUseProgram(shaderProgram[ShaderProgram::DepthVisualization]);

    // Send in the required data
    glm::vec4 data = glm::vec4(mainWindow.width, mainWindow.height, msaaLevel, mode);
    glUniform4fv(0, 1, glm::value_ptr(data));
    glm::vec2 clipPlanes = glm::vec2(nearClipPlane, farClipPlane);
    glUniform2fv(1, 1, glm::value_ptr(clipPlanes));

    // Bind the required textures
    GLenum target = (msaaLevel > 1) ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(target, renderTarget);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(target, viewSpacePos);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(target, depthStencil);

    // Draw fullscreen quad
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
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

  // Enter the application main loop
  mainLoop();

  // Release used resources and exit
  shutDown();
  return 0;
}
