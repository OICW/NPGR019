/*
 * Source code for the NPGR019 lab practices. Copyright Martin Kahoun 2021.
 * Licensed under the zlib license, see LICENSE.txt in the root directory.
 */

#include <Textures.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

Textures::Textures() : _samplers{0}
{
  stbi_set_flip_vertically_on_load(true);
}

Textures::~Textures()
{
  // Release samplers
  glDeleteSamplers((GLsizei)Sampler::NumSamplers, _samplers);
}

Textures& Textures::GetInstance()
{
  static Textures instance;
  return instance;
}

GLuint Textures::CreateCheckerBoardTexture(unsigned int textureSize, unsigned int checkerSize, glm::vec3 oddColor, glm::vec3 evenColor, bool sRGB)
{
  // Generate the texture name
  GLuint tex;
  glGenTextures(1, &tex);

  // Create the texture object (first bind call for this name)
  glBindTexture(GL_TEXTURE_2D, tex);

  // Generate texture RGB data
  const int stride = 3;
  unsigned char *data = new unsigned char[stride * textureSize * textureSize];
  for (unsigned int y = 0; y < textureSize; ++y)
  {
    for (unsigned int x = 0; x < textureSize; ++x)
    {
      const bool odd = ((x / checkerSize + y / checkerSize) & 1) > 0;
      unsigned char r = (unsigned char)((odd ? oddColor.x : evenColor.x) * 255.0f + 0.5f);
      unsigned char g = (unsigned char)((odd ? oddColor.y : evenColor.y) * 255.0f + 0.5f);
      unsigned char b = (unsigned char)((odd ? oddColor.z : evenColor.z) * 255.0f + 0.5f);

      int i = y * stride * textureSize + x * stride;
      data[i] = r;
      data[i + 1] = g;
      data[i + 2] = b;
    }
  }

  // Upload texture data: 2D texture, mip level 0, internal format RGB, width, height, border, input format RGB, type, data
  glTexImage2D(GL_TEXTURE_2D, 0, sRGB ? GL_SRGB : GL_RGB, textureSize, textureSize, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);

  // Unbind the texture
  glBindTexture(GL_TEXTURE_2D, 0);

  // Delete the temporary buffer
  delete[] data;

  // Note: the caller is now responsible for handling this resource
  return tex;
}

GLuint Textures::CreateSingleColorTexture(unsigned char r, unsigned char g, unsigned char b)
{
  // Generate the texture name
  GLuint tex;
  glGenTextures(1, &tex);

  // Create the texture object (first bind call for this name)
  glBindTexture(GL_TEXTURE_2D, tex);

  unsigned char data[] = {r, g, b};

  // Upload texture data: 2D texture, mip level 0, internal format RGB, width, height, border, input format RGB, type, data
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

  // Unbind the texture
  glBindTexture(GL_TEXTURE_2D, 0);

  return tex;
}

GLuint Textures::CreateMipMapTestTexture()
{  // Generate the texture name
  GLuint tex;
  glGenTextures(1, &tex);

  // Create the texture object (first bind call for this name)
  glBindTexture(GL_TEXTURE_2D, tex);

  const unsigned int stride = 3;
  unsigned char colors[] = {
    255, 0, 0,
    0, 255, 0,
    0, 0, 255,
    255, 255, 0,
    255, 0, 255,
    0, 255, 255,
    255, 255, 255,
    127, 127, 127,
    0, 0, 0
  };

  for (unsigned int size = 256, mip = 0; size > 0; size >>= 1, ++mip)
  {
    // Create intermediate buffer and fill it with colors
    unsigned char *data = new unsigned char[stride * size * size];
    for (unsigned int y = 0; y < size; ++y)
    {
      for (unsigned int x = 0; x < size; ++x)
      {
        int i = y * stride * size + x * stride;
        data[i] = colors[mip * stride];
        data[i + 1] = colors[mip * stride + 1];
        data[i + 2] = colors[mip * stride + 2];
      }
    }

    // Upload texture data: 2D texture, mip level, internal format RGB, width, height, border, input format RGB, type, data
    glTexImage2D(GL_TEXTURE_2D, mip, GL_RGB, size, size, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

    // Delete the intermediate buffer
    delete[] data;
  }

  // Unbind the texture
  glBindTexture(GL_TEXTURE_2D, 0);

  return tex;
}

GLuint Textures::LoadTexture(const char name[], bool sRGB)
{
  // Load stored texture on the disk
  int width, height, numChannels;
  unsigned char *data = stbi_load(name, &width, &height, &numChannels, 0);

  // Early return when we failed to load the texture
  if (!data)
  {
    printf("Failed to load texture: %s\n", name);
    return 0;
  }

  // Generate the texture name
  GLuint tex;
  glGenTextures(1, &tex);

  // Create the texture object (first bind call for this name)
  glBindTexture(GL_TEXTURE_2D, tex);

  // Upload texture data: 2D texture, mip level 0, internal format RGB, width, height, border, input format RGB, type, data
  glTexImage2D(GL_TEXTURE_2D, 0, sRGB ? GL_SRGB : GL_RGB, width, height, 0, numChannels == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);

  // Free the image data, we don't need them anymore
  stbi_image_free(data);

  // Unbind the texture
  glBindTexture(GL_TEXTURE_2D, 0);

  return tex;
}

void Textures::CreateSamplers()
{
  // Generate symbolic names for all samplers
  glGenSamplers((GLsizei)Sampler::NumSamplers, _samplers);

  // Query max anisotropy level - should be 16 on modern HW
  float maxAnistropy;
  glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAnistropy);

  // Set filtering modes:
  // Nearest neighbour or point filtering
  glSamplerParameteri(_samplers[(int)Sampler::Nearest], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glSamplerParameteri(_samplers[(int)Sampler::Nearest], GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  // Bilienar filtering, don't care about mip-maps
  glSamplerParameteri(_samplers[(int)Sampler::Bilinear], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glSamplerParameteri(_samplers[(int)Sampler::Bilinear], GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  // Trilinear filtering, do bilinar samples in two nearest mip-maps, linearly filter between the two samples
  glSamplerParameteri(_samplers[(int)Sampler::Trilinear], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glSamplerParameteri(_samplers[(int)Sampler::Trilinear], GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

  // Use mip-maps and anisotropic filter to obtain maxAnisotropy samples
  glSamplerParameteri(_samplers[(int)Sampler::Anisotropic], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glSamplerParameteri(_samplers[(int)Sampler::Anisotropic], GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glSamplerParameterf(_samplers[(int)Sampler::Anisotropic], GL_TEXTURE_MAX_ANISOTROPY, maxAnistropy);

  // Same as above
  glSamplerParameteri(_samplers[(int)Sampler::AnisotropicClamp], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glSamplerParameteri(_samplers[(int)Sampler::AnisotropicClamp], GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glSamplerParameterf(_samplers[(int)Sampler::AnisotropicClamp], GL_TEXTURE_MAX_ANISOTROPY, maxAnistropy);

  // Same as above
  glSamplerParameteri(_samplers[(int)Sampler::AnisotropicMirrored], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glSamplerParameteri(_samplers[(int)Sampler::AnisotropicMirrored], GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glSamplerParameterf(_samplers[(int)Sampler::AnisotropicMirrored], GL_TEXTURE_MAX_ANISOTROPY, maxAnistropy);

  // --------------------------------------------------------------------------

  // Set texture addressing mode:
  // Want to have the first modes to repeat outside [0, 1] range
  for (int i = 0; i <= (int)Sampler::Anisotropic; ++i)
  {
    glSamplerParameteri(_samplers[i], GL_TEXTURE_WRAP_S, GL_REPEAT);
    glSamplerParameteri(_samplers[i], GL_TEXTURE_WRAP_T, GL_REPEAT);
  }

  // This one should clamp the texture to edge
  glSamplerParameteri(_samplers[(int)Sampler::AnisotropicClamp], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glSamplerParameteri(_samplers[(int)Sampler::AnisotropicClamp], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  // This one should repeat but in mirrored fashion
  glSamplerParameteri(_samplers[(int)Sampler::AnisotropicMirrored], GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
  glSamplerParameteri(_samplers[(int)Sampler::AnisotropicMirrored], GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
}
