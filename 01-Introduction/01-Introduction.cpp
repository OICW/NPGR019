/*
 * Source code for the NPGR019 lab practices. Copyright Martin Kahoun 2021.
 * Licensed under the zlib license, see LICENSE.txt in the root directory.
 */

#include <cstdio>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define _USE_BUFFERS 0
#define _INTERLEAVED_BUFFER 0

// ----------------------------------------------------------------------------

// Vertex shader sources
static const char* vsSource[] = {
// ----------------------------------------------------------------------------
// Vertex shader with hardcoded triangle coordinates
// ----------------------------------------------------------------------------
R"(
#version 330 core

vec3 positions[3] = vec3[3](vec3(-0.25f, -0.25f, 0.0f),
                            vec3( 0.25f, -0.25f, 0.0f),
                            vec3( 0.25f,  0.25f, 0.0f));

vec3 colors[3] = vec3[3](vec3(1.0f, 0.0f, 0.0f),
                         vec3(0.0f, 1.0f, 0.0f),
                         vec3(0.0f, 0.0f, 1.0f));

out vec3 vColor;

void main()
{
  vColor = colors[gl_VertexID].rgb;
  gl_Position = vec4(positions[gl_VertexID].xyz, 1.0f);
}
)",
// ----------------------------------------------------------------------------
// Vertex shader accepting positions and colors from a buffer
// ----------------------------------------------------------------------------
R"(
#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

out vec3 vColor;

void main()
{
  vColor = color;
  gl_Position = vec4(position.xyz, 1.0f);
}
)"
};

// Fragment shader sources
static const char* fsSource[] = {
R"(
#version 330 core

in vec3 vColor;
out vec4 color;

void main()
{
  color = vec4(vColor.rgb, 1.0f);
}
)", "" };

// ----------------------------------------------------------------------------

// Enum class for storing default window parameters
enum class WindowParams : int
{
  Width = 800,
  Height = 600
};

// ----------------------------------------------------------------------------

// Max buffer length
static const unsigned int MAX_BUFFER_LENGTH = 256;
// Main window handle
GLFWwindow* mainWindow = nullptr;
// Shader program handle
GLuint shaderProgram = 0;
// Vertex Array Object handle (i.e., vertex buffer)
GLuint vertexArrayObject = 0;

#if _USE_BUFFERS
#if _INTERLEAVED_BUFFER
// Vertex buffer holding the data
GLuint vertexBuffer = 0;
#else
// Buffer holding the positional data
GLuint positionBuffer = 0;
// Buffer holding the color data
GLuint colorBuffer = 0;
#endif // _INTERLEAVED_BUFFER
#endif // _USE_BUFFERS

// ----------------------------------------------------------------------------

// Callback for handling GLFW errors
void errorCallback(int error, const char* description)
{
  printf("GLFW Error %i: %s\n", error, description);
}

// Callback for handling window resize events
void resizeCallback(GLFWwindow* window, int width, int height)
{
  glViewport(0, 0, width, height);
}

// Helper function for creating and compiling the shaders
bool compileShaders()
{
  GLuint vertexShader, fragmentShader;
  GLint result;
  GLchar log[MAX_BUFFER_LENGTH];

  // Create and compile the vertex shader
  vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, vsSource + _USE_BUFFERS, nullptr);
  glCompileShader(vertexShader);

  // Check that compilation was a success
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
  if (result == GL_FALSE)
  {
    glGetShaderInfoLog(vertexShader, MAX_BUFFER_LENGTH, nullptr, log);
    printf("Vertex shader compilation failed: %s\n", log);
    return false;
  }

  // Create and compile the fragment shader
  fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, fsSource, nullptr);
  glCompileShader(fragmentShader);

  // Check that compilation was a success
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
  if (result == GL_FALSE)
  {
    glGetShaderInfoLog(fragmentShader, MAX_BUFFER_LENGTH, nullptr, log);
    printf("Fragment shader compilation failed: %s\n", log);
    glDeleteShader(vertexShader);
    return false;
  }

  // Create the shader program, attach shaders, link
  shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);

  // Check that linkage was a success
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &result);
  if (result == GL_FALSE)
  {
    glGetProgramInfoLog(shaderProgram, MAX_BUFFER_LENGTH, nullptr, log);
    printf("Shader program linking failed: %s\n", log);

    // Note that we dispose of the shaderProgram later on in the shutDown() function
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return false;
  }

  // Clean up resources we don't need anymore at this point
  glDetachShader(shaderProgram, vertexShader);
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
  glDetachShader(shaderProgram, fragmentShader);

  return true;
}

// Helper method for creating scene geometry
void createGeometry()
{
  // Create and bind the VAO
  glGenVertexArrays(1, &vertexArrayObject);
  glBindVertexArray(vertexArrayObject);

#if _USE_BUFFERS
  // Data description, ideally, you'd create a structure
  const size_t VertexCount = 6;
  const size_t PositionDim = 3;
  const size_t ColorDim = 3;

#if _INTERLEAVED_BUFFER
  // Interleaved buffer with positions and colors
  float vertices[] = {-0.25f, -0.25f, 0.0f, // position of the 1st vertex (XYZ)
                       1.0f,   0.0f,  0.0f, // color of the 1st vertex (RGB)
                       0.25f, -0.25f, 0.0f, // position of the 2nd vertex (XYZ)
                        0.0f,  1.0f,  0.0f, // color of the 2nd vertex (RGB)
                       0.25f,  0.25f, 0.0f, // ...
                        0.0f,  0.0f,  1.0f,
                      -0.25f,  0.25f, 0.0f,
                        1.0f,  0.0f,  1.0f,
                      -0.25f, -0.25f, 0.0f,
                        1.0f,  1.0f,  0.0f,
                       0.25f,  0.25f, 0.0f,
                        0.0f,  1.0f,  1.0f};

  // Generate the memory storage
  glGenBuffers(1, &vertexBuffer);

  // Fill the buffer with the data: we're passing 6 vertices each composed of 2 vec3 values => sizeof(float) * 2 * 3 * 6
  // GL_STATIC_DRAW tells the driver that we don't intent to change the buffer afterwards and want only to draw from it
  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * (PositionDim + ColorDim) * VertexCount, vertices, GL_STATIC_DRAW);

  // Now, we need to tell OpenGL where to find the data using interleaved buffer: X0,Y0,Z0,R0,G0,B0,X1,Y1,Z1,R1,G1,B1,...
  // Stride - how many bytes to skip before the next vertex attribute
  // Offset - how many bytes to skip for the first attribute of that type

  // Positions: 3 floats, stride = 6 (6 floats/vertex), offset = 0
  glVertexAttribPointer(0, PositionDim, GL_FLOAT, GL_FALSE, (PositionDim + ColorDim) * sizeof(float), reinterpret_cast<void*>(0));
  glEnableVertexAttribArray(0); // Tied to the location in the shader

  // Colors: 3 floats, stride = 6 (6 floats/vertex), offset = 3 floats, i.e., 3 * 4 bytes
  glVertexAttribPointer(1, ColorDim, GL_FLOAT, GL_FALSE, (PositionDim + ColorDim) * sizeof(float), reinterpret_cast<void*>(PositionDim * sizeof(float)));
  glEnableVertexAttribArray(1); // Tied to the location in the shader

  // Unbind the buffer
  glBindBuffer(GL_ARRAY_BUFFER, 0);
#else
  // Buffer with positions
  float positions[] = {-0.25f, -0.25f, 0.0f, // position of the 1st vertex (XYZ)
                        0.25f, -0.25f, 0.0f, // position of the 2nd vertex (XYZ)
                        0.25f,  0.25f, 0.0f, // ...
                       -0.25f,  0.25f, 0.0f,
                       -0.25f, -0.25f, 0.0f,
                        0.25f,  0.25f, 0.0f};

  // Buffer with colors
  unsigned char colors[] = { 255,   0,   0, // color of the 1st vertex (RGB)
                               0, 255,   0, // color of the 2nd vertex (RGB)
                               0,   0, 255, // ...
                             255,   0, 255,
                             255, 255,   0,
                               0, 255, 255};

  // Generate the memory storage
  glGenBuffers(1, &positionBuffer);
  glGenBuffers(1, &colorBuffer);

  // Fill the buffer with the data: we're passing 6 vertices each composed of vec3 values => sizeof(float) * 3 * 6
  glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * PositionDim * VertexCount, positions, GL_STATIC_DRAW);

  // Positions: 3 floats, stride = 3 (3 floats/vertex), offset = 0
  glVertexAttribPointer(0, PositionDim, GL_FLOAT, GL_FALSE, PositionDim * sizeof(float), reinterpret_cast<void*>(0));
  glEnableVertexAttribArray(0); // Tied to the location in the shader

  // Unbind the buffer
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Fill the buffer with the data: we're passing 6 vertices each composed of vec3 values => sizeof(char) * 3 * 6
  glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(char) * ColorDim * VertexCount, colors, GL_STATIC_DRAW);

  // Colors: 3 chars, stride = 3 (3 char/vertex), offset = 0
  glVertexAttribPointer(1, ColorDim, GL_UNSIGNED_BYTE, GL_TRUE, ColorDim * sizeof(char), reinterpret_cast<void*>(0));
  glEnableVertexAttribArray(1); // Tied to the location in the shader

  // Unbind the buffer
  glBindBuffer(GL_ARRAY_BUFFER, 0);
#endif // _INTERLEAVED_BUFFER
#endif // _USE_BUFFERS
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
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // Create the window
  mainWindow = glfwCreateWindow((int)WindowParams::Width, (int)WindowParams::Height, "NPGR019 - Hello Triangle", nullptr, nullptr);
  if (mainWindow == nullptr)
  {
    printf("Failed to create the GLFW window!");
    return false;
  }

  // Make the created window with OpenGL context current for this thread
  glfwMakeContextCurrent(mainWindow);

  // Check that GLAD .dll loader and symbol imported is ready
  if (!gladLoadGL()) {
    printf("GLAD failed!\n");
    return false;
  }

  // Register a window resize callback
  glfwSetFramebufferSizeCallback(mainWindow, resizeCallback);

  // Set the OpenGL viewport
  glViewport(0, 0, (int)WindowParams::Width, (int)WindowParams::Height);

  return true;
}

// Helper method for graceful shutdown
void shutDown()
{
  // Release the vertex array object
  glDeleteVertexArrays(1, &vertexArrayObject);

  // Delete vertex buffers
#if _USE_BUFFERS
#if _INTERLEAVED_BUFFER
  glDeleteBuffers(1, &vertexBuffer);
#else
  glDeleteBuffers(1, &positionBuffer);
  glDeleteBuffers(1, &colorBuffer);
#endif // _INTERLEAVED_BUFFER
#endif // _USE_BUFFERS

  // Release the shader program
  glDeleteProgram(shaderProgram);

  // Release the window
  glfwDestroyWindow(mainWindow);

  // Close the GLFW library
  glfwTerminate();
}

// Helper method for handling input events
void processInput()
{
  // Notify the window that user wants to exit the application
  if (glfwGetKey(mainWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(mainWindow, true);
}

void renderScene()
{
  // Clear the color buffer
  glClearColor(0.1f, 0.2f, 0.4f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  // Tell OpenGL we'd like to use the previously compiled shader program
  glUseProgram(shaderProgram);

  // Draw the scene geometry - just tell OpenGL we're drawing at this point
  glPointSize(10.0f);
  glDrawArrays(GL_TRIANGLES, 0, 6);

  // Unbind the shader program
  glUseProgram(0);
}

// Helper method for implementing the application main loop
void mainLoop()
{
  while (!glfwWindowShouldClose(mainWindow))
  {
    // Poll the events like keyboard, mouse, etc.
    glfwPollEvents();

    // Process keyboard input
    processInput();

    // Render the scene
    renderScene();

    // Swap actual buffers on the GPU
    glfwSwapBuffers(mainWindow);
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
