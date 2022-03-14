/*
 * Source code for the NPGR019 lab practices. Copyright Martin Kahoun 2021.
 * Licensed under the zlib license, see LICENSE.txt in the root directory.
 */

#pragma once

#include <glad/glad.h>
#include <vector>

// Class for mesh representation
template <class VertexType>
class Mesh
{
public:
  Mesh() : _vao(0), _vbo(0), _vboSize(0), _ibo(0), _iboSize(0) {}
  ~Mesh();

  // Initialize the mesh with data
  void Init(const std::vector<VertexType> &vb, const std::vector<GLuint> &ib);
  // Return the associated VAO for rendering
  GLuint GetVAO() { return _vao; }
  // Get the size of the vertex buffer
  GLsizei GetVBOSize() { return _vboSize; }
  // Get the size of the index buffer
  GLsizei GetIBOSize() { return _iboSize; }

protected:
  // Vertex array object used to draw this mesh
  GLuint _vao;
  // Vertex buffer
  GLuint _vbo;
  // Vertex buffer size in # of vertices
  GLsizei _vboSize;
  // Index buffer
  GLuint _ibo;
  // Index buffer size
  GLsizei _iboSize;

private:
  // No copies allowed
  Mesh(const Mesh &);
  Mesh & operator = (const Mesh &);
};

template <class VertexType>
Mesh<VertexType>::~Mesh()
{
  // Release resources used by the driver
  glDeleteVertexArrays(1, &_vao);
  glDeleteBuffers(1, &_vbo);
  glDeleteBuffers(1, &_ibo);
}

template<class VertexType>
void Mesh<VertexType>::Init(const std::vector<VertexType> &vb, const std::vector<GLuint> &ib)
{
  // Do nothing if we're already initialized
  if (_vao)
    return;

  _vboSize = (GLsizei)vb.size();
  _iboSize = (GLsizei)ib.size();

  // Create and bind the Vertex Array Object
  glGenVertexArrays(1, &_vao);
  glBindVertexArray(_vao);

  // Generate the vertex buffer fill it with data
  glGenBuffers(1, &_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, _vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(VertexType) * _vboSize, static_cast<const void *>(&*vb.begin()), GL_STATIC_DRAW);

  // Describe and enable vertex attributes
  VertexType::BindVertexAttributes();

  // Unbind the array buffer to prevent accidental changes
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Generate the index buffer and fill it with data
  glGenBuffers(1, &_ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * _iboSize, static_cast<const void *>(&*ib.begin()), GL_STATIC_DRAW);

  // Note: can't unbind the IBO while the VAO is active unlike VBO which got stored trough glVertexAttribPointer call, it would break things

  // Unbind the VAO
  glBindVertexArray(0);
}
