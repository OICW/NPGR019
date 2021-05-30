/*
 * Source code for the NPGR019 lab practices. Copyright Martin Kahoun 2021.
 * Licensed under the zlib license, see LICENSE.txt in the root directory.
 */

#include "Geometry.h"

#include <glm/glm.hpp>

Mesh<Vertex_Pos_Col> *Geometry::CreateQuadColor()
{
  // Create the vertex buffer for a quad
  std::vector<Vertex_Pos_Col> vb;
  vb.reserve(4);

  // Create vertices
  vb.push_back({-0.5f, 0.0f, -0.5f, 0.5f, 0.5f, 0.5f});
  vb.push_back({ 0.5f, 0.0f, -0.5f, 0.5f, 0.5f, 0.5f});
  vb.push_back({ 0.5f, 0.0f,  0.5f, 0.5f, 0.5f, 0.5f});
  vb.push_back({-0.5f, 0.0f,  0.5f, 0.5f, 0.5f, 0.5f});

  // Fill in the index buffer
  std::vector<GLuint> ib;
  ib.reserve(6);

  // One triangle
  ib.push_back(0);
  ib.push_back(1);
  ib.push_back(2);

  // Other triangle
  ib.push_back(2);
  ib.push_back(3);
  ib.push_back(0);

  // Create, initialize and return the mesh
  Mesh<Vertex_Pos_Col> *mesh = new Mesh<Vertex_Pos_Col>();
  mesh->Init(vb, ib);
  return mesh;
}

Mesh<Vertex_Pos_Tex> *Geometry::CreateQuadTex()
{
  // Create the vertex buffer for a quad
  std::vector<Vertex_Pos_Tex> vb;
  vb.reserve(4);

  // Create vertices
  vb.push_back({-0.5f, 0.0f, -0.5f, 0.0f, 0.0f});
  vb.push_back({ 0.5f, 0.0f, -0.5f, 1.0f, 0.0f});
  vb.push_back({ 0.5f, 0.0f,  0.5f, 1.0f, 1.0f});
  vb.push_back({-0.5f, 0.0f,  0.5f, 0.0f, 1.0f});

  // Fill in the index buffer
  std::vector<GLuint> ib;
  ib.reserve(6);

  // One triangle
  ib.push_back(0);
  ib.push_back(1);
  ib.push_back(2);

  // Other triangle
  ib.push_back(2);
  ib.push_back(3);
  ib.push_back(0);

  // Create, initialize and return the mesh
  Mesh<Vertex_Pos_Tex> *mesh = new Mesh<Vertex_Pos_Tex>();
  mesh->Init(vb, ib);
  return mesh;
}

Mesh<Vertex_Pos_Nrm_Tgt_Tex> *Geometry::CreateQuadNormalTangentTex()
{
  // Create the vertex buffer for a quad
  std::vector<Vertex_Pos_Nrm_Tgt_Tex> vb;
  vb.reserve(4);

  // Create vertices
  vb.push_back({-0.5f, 0.0f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f});
  vb.push_back({ 0.5f, 0.0f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f});
  vb.push_back({ 0.5f, 0.0f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f});
  vb.push_back({-0.5f, 0.0f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f});

  // Fill in the index buffer
  std::vector<GLuint> ib;
  ib.reserve(6);

  // One triangle
  ib.push_back(0);
  ib.push_back(1);
  ib.push_back(2);

  // Other triangle
  ib.push_back(2);
  ib.push_back(3);
  ib.push_back(0);

  // Create, initialize and return the mesh
  Mesh<Vertex_Pos_Nrm_Tgt_Tex> *mesh = new Mesh<Vertex_Pos_Nrm_Tgt_Tex>();
  mesh->Init(vb, ib);
  return mesh;
}

Mesh<Vertex_Pos_Col> *Geometry::CreateCubeColor()
{
  // Create the vertex buffer for a unit cube
  std::vector<Vertex_Pos_Col> vb;
  vb.reserve(24);

  // Top face
  vb.push_back({-0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f});
  vb.push_back({ 0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f});
  vb.push_back({ 0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f});
  vb.push_back({-0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f});

  // Bottom face
  vb.push_back({ 0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 1.0f});
  vb.push_back({-0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 1.0f});
  vb.push_back({-0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 1.0f});
  vb.push_back({ 0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 1.0f});

  // Front face
  vb.push_back({-0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f});
  vb.push_back({ 0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f});
  vb.push_back({ 0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 1.0f});
  vb.push_back({-0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 1.0f});

  // Back face
  vb.push_back({ 0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f});
  vb.push_back({-0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f});
  vb.push_back({-0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f});
  vb.push_back({ 0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f});

  // Left face
  vb.push_back({-0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 0.0f});
  vb.push_back({-0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f});
  vb.push_back({-0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 0.0f});
  vb.push_back({-0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 0.0f});

  // Right face
  vb.push_back({0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f});
  vb.push_back({0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f});
  vb.push_back({0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f});
  vb.push_back({0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f});

  // Fill in the index buffer
  std::vector<GLuint> ib;
  ib.reserve(36);
  for (int face = 0; face < 6; ++face)
  {
    GLuint baseIndex = 4 * face;

    // One triangle
    ib.push_back(baseIndex);
    ib.push_back(baseIndex + 1);
    ib.push_back(baseIndex + 2);

    // Other triangle
    ib.push_back(baseIndex + 2);
    ib.push_back(baseIndex + 3);
    ib.push_back(baseIndex);
  }

  // Create, initialize and return the mesh
  Mesh<Vertex_Pos_Col> *mesh = new Mesh<Vertex_Pos_Col>();
  mesh->Init(vb, ib);
  return mesh;
}

Mesh<Vertex_Pos_Col> *Geometry::CreateCubeColorShared()
{
  // Create the vertex buffer for a unit cube
  std::vector<Vertex_Pos_Col> vb;
  vb.reserve(8);

  // Top base
  vb.push_back({-0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f});
  vb.push_back({ 0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 0.0f});
  vb.push_back({ 0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f});
  vb.push_back({-0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 1.0f});

  // Bottom base
  vb.push_back({ 0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f});
  vb.push_back({-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.0f});
  vb.push_back({-0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f});
  vb.push_back({ 0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 1.0f});

  // Fill in the index buffer
  std::vector<GLuint> ib;
  ib.reserve(36);

  // Top face
  ib.push_back(0);
  ib.push_back(1);
  ib.push_back(2);
  ib.push_back(2);
  ib.push_back(3);
  ib.push_back(0);

  // Bottom face
  ib.push_back(4);
  ib.push_back(5);
  ib.push_back(6);
  ib.push_back(6);
  ib.push_back(7);
  ib.push_back(4);

  // Front face
  ib.push_back(5);
  ib.push_back(4);
  ib.push_back(1);
  ib.push_back(1);
  ib.push_back(0);
  ib.push_back(5);

  // Back face
  ib.push_back(7);
  ib.push_back(6);
  ib.push_back(3);
  ib.push_back(3);
  ib.push_back(2);
  ib.push_back(7);

  // Left face
  ib.push_back(6);
  ib.push_back(5);
  ib.push_back(0);
  ib.push_back(0);
  ib.push_back(3);
  ib.push_back(6);

  // Right face
  ib.push_back(4);
  ib.push_back(7);
  ib.push_back(2);
  ib.push_back(2);
  ib.push_back(1);
  ib.push_back(4);

  // Create, initialize and return the mesh
  Mesh<Vertex_Pos_Col> *mesh = new Mesh<Vertex_Pos_Col>();
  mesh->Init(vb, ib);
  return mesh;
}

Mesh<Vertex_Pos> *Geometry::CreateCubeAdjacency()
{
  // Create the vertex buffer for a unit cube
  std::vector<Vertex_Pos> vb;
  vb.reserve(8);

  // Top base
  vb.push_back({-0.5f,  0.5f, -0.5f});
  vb.push_back({ 0.5f,  0.5f, -0.5f});
  vb.push_back({ 0.5f,  0.5f,  0.5f});
  vb.push_back({-0.5f,  0.5f,  0.5f});

  // Bottom base
  vb.push_back({ 0.5f, -0.5f, -0.5f});
  vb.push_back({-0.5f, -0.5f, -0.5f});
  vb.push_back({-0.5f, -0.5f,  0.5f});
  vb.push_back({ 0.5f, -0.5f,  0.5f});

  // Fill in the index buffer
  std::vector<GLuint> ib;
  ib.reserve(72);

  // Top face
  ib.push_back(0);
  ib.push_back(5); // Adjacent vertex
  ib.push_back(1);
  ib.push_back(4); // Adjacent vertex
  ib.push_back(2);
  ib.push_back(3); // Adjacent vertex

  ib.push_back(2);
  ib.push_back(7); // Adjacent vertex
  ib.push_back(3);
  ib.push_back(6); // Adjacent vertex
  ib.push_back(0);
  ib.push_back(1); // Adjacent vertex

  // Bottom face
  ib.push_back(4);
  ib.push_back(1); // Adjacent vertex
  ib.push_back(5);
  ib.push_back(0); // Adjacent vertex
  ib.push_back(6);
  ib.push_back(7); // Adjacent vertex

  ib.push_back(6);
  ib.push_back(3); // Adjacent vertex
  ib.push_back(7);
  ib.push_back(2); // Adjacent vertex
  ib.push_back(4);
  ib.push_back(5); // Adjacent vertex

  // Front face
  ib.push_back(5);
  ib.push_back(6); // Adjacent vertex
  ib.push_back(4);
  ib.push_back(2); // Adjacent vertex
  ib.push_back(1);
  ib.push_back(0); // Adjacent vertex

  ib.push_back(1);
  ib.push_back(2); // Adjacent vertex
  ib.push_back(0);
  ib.push_back(6); // Adjacent vertex
  ib.push_back(5);
  ib.push_back(4); // Adjacent vertex

  // Back face
  ib.push_back(7);
  ib.push_back(4); // Adjacent vertex
  ib.push_back(6);
  ib.push_back(0); // Adjacent vertex
  ib.push_back(3);
  ib.push_back(2); // Adjacent vertex

  ib.push_back(3);
  ib.push_back(0); // Adjacent vertex
  ib.push_back(2);
  ib.push_back(4); // Adjacent vertex
  ib.push_back(7);
  ib.push_back(6); // Adjacent vertex

  // Left face
  ib.push_back(6);
  ib.push_back(4); // Adjacent vertex
  ib.push_back(5);
  ib.push_back(1); // Adjacent vertex
  ib.push_back(0);
  ib.push_back(3); // Adjacent vertex

  ib.push_back(0);
  ib.push_back(2); // Adjacent vertex
  ib.push_back(3);
  ib.push_back(7); // Adjacent vertex
  ib.push_back(6);
  ib.push_back(5); // Adjacent vertex

  // Right face
  ib.push_back(4);
  ib.push_back(6); // Adjacent vertex
  ib.push_back(7);
  ib.push_back(3); // Adjacent vertex
  ib.push_back(2);
  ib.push_back(1); // Adjacent vertex

  ib.push_back(2);
  ib.push_back(0); // Adjacent vertex
  ib.push_back(1);
  ib.push_back(5); // Adjacent vertex
  ib.push_back(4);
  ib.push_back(7); // Adjacent vertex

  // Create, initialize and return the mesh
  Mesh<Vertex_Pos> *mesh = new Mesh<Vertex_Pos>();
  mesh->Init(vb, ib);
  return mesh;
}

Mesh<Vertex_Pos_Tex> *Geometry::CreateCubeTex()
{
  // Create the vertex buffer for a unit cube
  std::vector<Vertex_Pos_Tex> vb;
  vb.reserve(24);

  // Top face
  vb.push_back({-0.5f,  0.5f, -0.5f, 1.0f, 0.0f});
  vb.push_back({ 0.5f,  0.5f, -0.5f, 1.0f, 1.0f});
  vb.push_back({ 0.5f,  0.5f,  0.5f, 0.0f, 1.0f});
  vb.push_back({-0.5f,  0.5f,  0.5f, 0.0f, 0.0f});

  // Bottom face
  vb.push_back({ 0.5f, -0.5f, -0.5f, 1.0f, 0.0f});
  vb.push_back({-0.5f, -0.5f, -0.5f, 1.0f, 1.0f});
  vb.push_back({-0.5f, -0.5f,  0.5f, 0.0f, 1.0f});
  vb.push_back({ 0.5f, -0.5f,  0.5f, 0.0f, 0.0f});

  // Front face
  vb.push_back({-0.5f, -0.5f, -0.5f, 0.0f, 0.0f});
  vb.push_back({ 0.5f, -0.5f, -0.5f, 1.0f, 0.0f});
  vb.push_back({ 0.5f,  0.5f, -0.5f, 1.0f, 1.0f});
  vb.push_back({-0.5f,  0.5f, -0.5f, 0.0f, 1.0f});

  // Back face
  vb.push_back({ 0.5f, -0.5f,  0.5f, 0.0f, 0.0f});
  vb.push_back({-0.5f, -0.5f,  0.5f, 1.0f, 0.0f});
  vb.push_back({-0.5f,  0.5f,  0.5f, 1.0f, 1.0f});
  vb.push_back({ 0.5f,  0.5f,  0.5f, 0.0f, 1.0f});

  // Left face
  vb.push_back({-0.5f, -0.5f,  0.5f, 0.0f, 0.0f});
  vb.push_back({-0.5f, -0.5f, -0.5f, 1.0f, 0.0f});
  vb.push_back({-0.5f,  0.5f, -0.5f, 1.0f, 1.0f});
  vb.push_back({-0.5f,  0.5f,  0.5f, 0.0f, 1.0f});

  // Right face
  vb.push_back({ 0.5f, -0.5f, -0.5f, 0.0f, 0.0f});
  vb.push_back({ 0.5f, -0.5f,  0.5f, 1.0f, 0.0f});
  vb.push_back({ 0.5f,  0.5f,  0.5f, 1.0f, 1.0f});
  vb.push_back({ 0.5f,  0.5f, -0.5f, 0.0f, 1.0f});

  // Fill in the index buffer
  std::vector<GLuint> ib;
  ib.reserve(36);
  for (int face = 0; face < 6; ++face)
  {
    GLuint baseIndex = 4 * face;

    // One triangle
    ib.push_back(baseIndex);
    ib.push_back(baseIndex + 1);
    ib.push_back(baseIndex + 2);

    // Other triangle
    ib.push_back(baseIndex + 2);
    ib.push_back(baseIndex + 3);
    ib.push_back(baseIndex);
  }

  // Create, initialize and return the mesh
  Mesh<Vertex_Pos_Tex> *mesh = new Mesh<Vertex_Pos_Tex>();
  mesh->Init(vb, ib);
  return mesh;
}

Mesh<Vertex_Pos_Nrm_Tgt_Tex> *Geometry::CreateCubeNormalTangentTex()
{
  // Create the vertex buffer for a unit cube
  std::vector<Vertex_Pos_Nrm_Tgt_Tex> vb;
  vb.reserve(24);

  // Top face
  vb.push_back({-0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f}); // 0
  vb.push_back({ 0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f}); // 1
  vb.push_back({ 0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f}); // 2
  vb.push_back({-0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f}); // 3

  // Bottom face
  vb.push_back({ 0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f}); // 4
  vb.push_back({-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f}); // 5
  vb.push_back({-0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f}); // 6
  vb.push_back({ 0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f}); // 7

  // Front face
  vb.push_back({-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f}); // 8
  vb.push_back({ 0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f}); // 9
  vb.push_back({ 0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f}); // 10
  vb.push_back({-0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f}); // 11

  // Back face
  vb.push_back({ 0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f}); // 12
  vb.push_back({-0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f}); // 13
  vb.push_back({-0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f}); // 14
  vb.push_back({ 0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f}); // 15

  // Left face
  vb.push_back({-0.5f, -0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f}); // 16
  vb.push_back({-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f}); // 17
  vb.push_back({-0.5f,  0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f}); // 18
  vb.push_back({-0.5f,  0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f}); // 19

  // Right face
  vb.push_back({0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f}); // 20
  vb.push_back({0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f}); // 21
  vb.push_back({0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f}); // 22
  vb.push_back({0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f}); // 23

  // Fill in the index buffer
  std::vector<GLuint> ib;
  ib.reserve(36);
  for (int face = 0; face < 6; ++face)
  {
    GLuint baseIndex = 4 * face;

    // One triangle
    ib.push_back(baseIndex);
    ib.push_back(baseIndex + 1);
    ib.push_back(baseIndex + 2);

    // Other triangle
    ib.push_back(baseIndex + 2);
    ib.push_back(baseIndex + 3);
    ib.push_back(baseIndex);
  }

  // Create, initialize and return the mesh
  Mesh<Vertex_Pos_Nrm_Tgt_Tex> *mesh = new Mesh<Vertex_Pos_Nrm_Tgt_Tex>();
  mesh->Init(vb, ib);
  return mesh;
}

Mesh<Vertex_Pos_Nrm> *Geometry::CreateTetrahedron()
{
  // Create the vertex buffer for a tetrahedron
  std::vector<Vertex_Pos_Nrm> vb;
  vb.reserve(4);

  // Define vertices
  glm::vec3 v0 = glm::vec3(-0.5f, -0.3f, -0.5f);
  glm::vec3 v1 = glm::vec3( 0.5f, -0.3f, -0.5f);
  glm::vec3 v2 = glm::vec3( 0.0f, -0.3f,  1.5f);
  glm::vec3 v3 = glm::vec3( 0.0f,  0.2f,  0.0f);

  // Calculate edges
  glm::vec3 e0 = v1 - v0;
  glm::vec3 e1 = v2 - v0;
  glm::vec3 e2 = v3 - v0;
  glm::vec3 e3 = v3 - v1;
  glm::vec3 e4 = v2 - v1;

  // Calculate face normals (bottom, left, back, right)
  glm::vec3 n0 = glm::normalize(glm::cross(e0, e1));
  glm::vec3 n1 = glm::normalize(glm::cross(e1, e2));
  glm::vec3 n2 = glm::normalize(glm::cross(e2, e0));
  glm::vec3 n3 = glm::normalize(glm::cross(e3, e4));

  // Bottom face
  vb.push_back({v0.x, v0.y, v0.z, n0.x, n0.y, n0.z}); // 0
  vb.push_back({v2.x, v2.y, v2.z, n0.x, n0.y, n0.z}); // 1
  vb.push_back({v1.x, v1.y, v1.z, n0.x, n0.y, n0.z}); // 2

  // Left face
  vb.push_back({v0.x, v0.y, v0.z, n1.x, n1.y, n1.z}); // 3
  vb.push_back({v3.x, v3.y, v3.z, n1.x, n1.y, n1.z}); // 4
  vb.push_back({v2.x, v2.y, v2.z, n1.x, n1.y, n1.z}); // 5

  // Back face
  vb.push_back({v0.x, v0.y, v0.z, n2.x, n2.y, n2.z}); // 6
  vb.push_back({v1.x, v1.y, v1.z, n2.x, n2.y, n2.z}); // 7
  vb.push_back({v3.x, v3.y, v3.z, n2.x, n2.y, n2.z}); // 8

  // Right face
  vb.push_back({v1.x, v1.y, v1.z, n3.x, n3.y, n3.z}); // 9
  vb.push_back({v2.x, v2.y, v2.z, n3.x, n3.y, n3.z}); // 10
  vb.push_back({v3.x, v3.y, v3.z, n3.x, n3.y, n3.z}); // 11

  // Fill in the index buffer
  std::vector<GLuint> ib;
  ib.reserve(12);
  for (int face = 0; face < 4; ++face)
  {
    GLuint baseIndex = 3 * face;

    ib.push_back(baseIndex);
    ib.push_back(baseIndex + 1);
    ib.push_back(baseIndex + 2);
  }

  // Create, initialize and return the mesh
  Mesh<Vertex_Pos_Nrm> *mesh = new Mesh<Vertex_Pos_Nrm>();
  mesh->Init(vb, ib);
  return mesh;
}

Mesh<Vertex_Pos> *Geometry::CreateIcosahedron()
{
  // Create the vertex buffer for an icosahedron
  std::vector<Vertex_Pos> vb;
  vb.reserve(12);

  // Assume centering around origin, edge length of 2 -> circumradius is sqrt(phi + 2) = 1.902
  // phi = (1 + sqrt(5)) / 2 (golden ratio), we want circumradius of 1, so we'll need to scale
  // down the resulting geometry by factor of 1.902
  const float scale = 1.902f;
  const float unit = 1.0f / scale;
  const float phi = 1.618f / scale;

  // Top half
  vb.push_back({-phi,  unit, 0.0f}); // 0
  vb.push_back({0.0f,  phi, -unit}); // 1
  vb.push_back({ phi,  unit, 0.0f}); // 2
  vb.push_back({0.0f,  phi,  unit}); // 3

  // Base plane at y = 0
  vb.push_back({-unit, 0.0f, -phi}); // 4
  vb.push_back({ unit, 0.0f, -phi}); // 5
  vb.push_back({ unit, 0.0f,  phi}); // 6
  vb.push_back({-unit, 0.0f,  phi}); // 7

  // Bottom half
  vb.push_back({-phi, -unit, 0.0f}); // 8
  vb.push_back({0.0f, -phi,  unit}); // 9
  vb.push_back({ phi, -unit, 0.0f}); // 10
  vb.push_back({0.0f, -phi, -unit}); // 11

  // Fill in the index buffer
  std::vector<GLuint> ib;
  ib.reserve(60);

  // Top half
  ib.push_back(0);
  ib.push_back(4);
  ib.push_back(1);

  ib.push_back(0);
  ib.push_back(1);
  ib.push_back(3);

  ib.push_back(0);
  ib.push_back(3);
  ib.push_back(7);

  ib.push_back(4);
  ib.push_back(5);
  ib.push_back(1);

  ib.push_back(6);
  ib.push_back(7);
  ib.push_back(3);

  ib.push_back(2);
  ib.push_back(6);
  ib.push_back(3);

  ib.push_back(2);
  ib.push_back(3);
  ib.push_back(1);

  ib.push_back(2);
  ib.push_back(1);
  ib.push_back(5);

  // Middle sides
  ib.push_back(0);
  ib.push_back(7);
  ib.push_back(8);

  ib.push_back(0);
  ib.push_back(8);
  ib.push_back(4);

  ib.push_back(2);
  ib.push_back(5);
  ib.push_back(10);

  ib.push_back(2);
  ib.push_back(10);
  ib.push_back(6);

  // Bottom half
  ib.push_back(8);
  ib.push_back(7);
  ib.push_back(9);

  ib.push_back(8);
  ib.push_back(9);
  ib.push_back(11);

  ib.push_back(8);
  ib.push_back(11);
  ib.push_back(4);

  ib.push_back(5);
  ib.push_back(4);
  ib.push_back(11);

  ib.push_back(7);
  ib.push_back(6);
  ib.push_back(9);

  ib.push_back(10);
  ib.push_back(9);
  ib.push_back(6);

  ib.push_back(10);
  ib.push_back(11);
  ib.push_back(9);

  ib.push_back(10);
  ib.push_back(5);
  ib.push_back(11);

  // Create, initialize and return the mesh
  Mesh<Vertex_Pos> *mesh = new Mesh<Vertex_Pos>();
  mesh->Init(vb, ib);
  return mesh;
}
