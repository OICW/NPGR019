/*
 * Source code for the NPGR019 lab practices. Copyright Martin Kahoun 2021.
 * Licensed under the zlib license, see LICENSE.txt in the root directory.
 */

#pragma once

#include <glad/glad.h>

// Simple class for compiling shaders
class ShaderCompiler
{
public:
  // Maximum length for logging purposes
  static const unsigned int MAX_LOG_LENGTH = 1024;

  // Compiles shader of a specified type
  static GLuint CompileShader(const char* source[], int index, GLenum type);
  // Links specified program
  static bool LinkProgram(GLuint program);
};