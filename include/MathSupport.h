/*
 * Source code for the NPGR019 lab practices. Copyright Martin Kahoun 2021.
 * Licensed under the zlib license, see LICENSE.txt in the root directory.
 */

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

static const float PI = 3.1415926535897932384626433832795f;
static const float PI_HALF = 1.5707963267948966192313216916398f;
static const float TWO_PI = 6.283185307179586476925286766559f;

// Fast transformation matrix inversion, assumes only rotation and translation
inline glm::mat4x4 fastMatrixInverse(const glm::mat4x4& matrix)
{
  glm::mat3x3 inv = glm::transpose(glm::mat3x3(matrix));
  return glm::mat4x4(glm::vec4(inv[0], 0.0f),
                     glm::vec4(inv[1], 0.0f),
                     glm::vec4(inv[2], 0.0f),
                     glm::vec4(-inv * glm::vec3(matrix[3]), 1.0f));
}

// Gets random number from the [min, max] range
inline float getRandom(float min, float max)
{
  return min + static_cast<float>(rand()) / (static_cast <float>(RAND_MAX / (max - min)));
}

// C++ type safe signum function
template <typename T>
inline int sign(T value)
{
  return (T(0) < value) - (value < T(0));
}

// Converts RGB values to luminuos intensity
inline float getLuminousIntensity(glm::vec3 color)
{
  return 0.2126f * color.x + 0.7152f * color.y + 0.0722f * color.z;
}
