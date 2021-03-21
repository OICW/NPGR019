/*
 * Source code for the NPGR019 lab practices. Copyright Martin Kahoun 2021.
 * Licensed under the zlib license, see LICENSE.txt in the root directory.
 */

#pragma once

#include <glad/glad.h>

// Vertex containing vec3 position
struct Vertex_Pos
{
  // Position
  float x, y, z;

  static void BindVertexAttributes()
  {
    // Positions: 3 floats, stride = 3 * sizeof(float), offset = 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_Pos), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);
  }
};

// Vertex containing vec3 position, vec3 color
struct Vertex_Pos_Col
{
  // Position
  float x, y, z;
  // Vertex color
  float r, g, b;

  static void BindVertexAttributes()
  {
    // Positions: 3 floats, stride = 6 * sizeof(float), offset = 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_Pos_Col), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);
    // Texture coordinates: 3 floats, stride = 6 * sizeof(float), offset = 3
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_Pos_Col), reinterpret_cast<void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
  }
};

// Vertex containing vec3 position, vec2 UV coords
struct Vertex_Pos_Tex
{
  // Position
  float x, y, z;
  // Texture coordinates
  float u, v;

  static void BindVertexAttributes()
  {
    // Positions: 3 floats, stride = 5 * sizeof(float), offset = 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_Pos_Tex), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);
    // Texture coordinates: 2 floats, stride = 5 * sizeof(float), offset = 3
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_Pos_Tex), reinterpret_cast<void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
  }
};

// Vertex containing vec3 position, vec3 normal
struct Vertex_Pos_Nrm
{
  // Position
  float x, y, z;
  // Normal
  float nx, ny, nz;

  static void BindVertexAttributes()
  {
    // Positions: 3 floats, stride = 6 * sizeof(float), offset = 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_Pos_Nrm), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);
    // Normals: 3 floats, stride = 6 * sizeof(float), offset = 3
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_Pos_Nrm), reinterpret_cast<void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
  }
};

// Vertex containing vec3 position, vec3 normal, vec3 tangent, vec2 UV coords
struct Vertex_Pos_Nrm_Tgt_Tex
{
  // Position
  float x, y, z;
  // Normal
  float nx, ny, nz;
  // Tangent
  float tx, ty, tz;
  // Texture coordinates
  float u, v;

  static void BindVertexAttributes()
  {
    // Positions: 3 floats, stride = 11 * sizeof(float), offset = 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_Pos_Nrm_Tgt_Tex), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);
    // Normals: 3 floats, stride = 11 * sizeof(float), offset = 3
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_Pos_Nrm_Tgt_Tex), reinterpret_cast<void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // Tangents: 3 floats, stride = 11 * sizeof(float), offset = 6
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_Pos_Nrm_Tgt_Tex), reinterpret_cast<void*>(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // Texture coordinates: 2 floats, stride = 11 * sizeof(float), offset = 9
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_Pos_Nrm_Tgt_Tex), reinterpret_cast<void*>(9 * sizeof(float)));
    glEnableVertexAttribArray(3);
  }
};
