/*
 * Source code for the NPGR019 lab practices. Copyright Martin Kahoun 2021.
 * Licensed under the zlib license, see LICENSE.txt in the root directory.
 */

#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

enum class Sampler : int
{
  Nearest, Bilinear, Trilinear, Anisotropic, AnisotropicClamp, AnisotropicMirrored, NumSamplers
};

// Class for handling texture and sampler creation
class Textures
{
public:
  // Get and create instance for this singleton
  static Textures& GetInstance();
  // Create checkerboard pattern texture
  static GLuint CreateCheckerBoardTexture(unsigned int textureSize, unsigned int checkerSize, glm::vec3 oddColor = glm::vec3(0.15f, 0.15f, 0.6f), glm::vec3 evenColor = glm::vec3(0.85f, 0.75f, 0.3f), bool sRGB = true);
  // Create single color texture for default usage
  static GLuint CreateSingleColorTexture(unsigned char r, unsigned char g, unsigned char b);
  // Create mip-map chain testing texture
  static GLuint CreateMipMapTestTexture();
  // Load texture from file stored on the disk
  static GLuint LoadTexture(const char name[], bool sRGB);
  // Create all samplers
  void CreateSamplers();
  // Get sampler
  GLuint GetSampler(Sampler sampler);

private:
  // All is private, instance is created in GetInstance()
  Textures();
  ~Textures();
  // No copies allowed
  Textures(const Textures &);
  Textures & operator = (const Textures &);

  // All available texture samplers
  GLuint _samplers[(int)Sampler::NumSamplers];
};

inline GLuint Textures::GetSampler(Sampler sampler)
{
  return _samplers[(int)sampler];
}
