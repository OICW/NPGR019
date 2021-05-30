/*
 * Source code for the NPGR019 lab practices. Copyright Martin Kahoun 2021.
 * Licensed under the zlib license, see LICENSE.txt in the root directory.
 */

#pragma once

#include "Mesh.h"
#include "Vertex.h"

// Geometry utilities class
class Geometry
{
public:
  // Creates simple quad with uniform color
  static Mesh<Vertex_Pos_Col> *CreateQuadColor();
  // Create simple quad with texture coordinates
  static Mesh<Vertex_Pos_Tex> *CreateQuadTex();
  // Create simple quad with normals, tangents and texture coordinates
  static Mesh<Vertex_Pos_Nrm_Tgt_Tex> *CreateQuadNormalTangentTex();
  // Creates simple cube with colors
  static Mesh<Vertex_Pos_Col> *CreateCubeColor();
  // Creates simple cube with colors and shared vertices
  static Mesh<Vertex_Pos_Col> *CreateCubeColorShared();
  // Creates simple cube with shared vertices and vertex adjacency info
  static Mesh<Vertex_Pos> *CreateCubeAdjacency();
  // Creates simple cube with texture coordinates
  static Mesh<Vertex_Pos_Tex> *CreateCubeTex();
  // Create simple cube with normals, tangents and texture coordinates
  static Mesh<Vertex_Pos_Nrm_Tgt_Tex> *CreateCubeNormalTangentTex();
  // Create tethrahedron composed from vertices with normals
  static Mesh<Vertex_Pos_Nrm> *CreateTetrahedron();
  // Create regular icosahedron with just positions
  static Mesh<Vertex_Pos> *CreateIcosahedron();

private:
  Geometry();
  ~Geometry();
};
