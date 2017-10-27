/*******************************************************************************
 * MIT License
 *
 * Copyright (c) 2017 Yuriy Khokhulya
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *******************************************************************************/

#include "CMesh.hpp"

namespace gles2 {

CMesh::CMesh(Vertices vertices, Indices indices)
  : m_vertices(vertices)
  , m_indices(indices)
  , m_vbuffer(GL_ARRAY_BUFFER, vertices)
  , m_ibuffer(GL_ELEMENT_ARRAY_BUFFER, indices)
{
}

void CMesh::draw(CShaderProgram& program, bool points)
{
  gles2::CBuffer::bind(m_vbuffer);
  gles2::CBuffer::bind(m_ibuffer);

  program.enableAttrArray("a_pos");
  program.enableAttrArray("a_tex0");
  program.setAttrBuffer("a_pos", GL_FLOAT, 3, 5 * sizeof(float), 0);
  program.setAttrBuffer(
      "a_tex0", GL_FLOAT, 2, 5 * sizeof(float), 3 * sizeof(float));

  glDrawElements(
      points ? GL_POINTS : GL_TRIANGLE_STRIP,
      m_indices.size(),
      GL_UNSIGNED_SHORT,
      nullptr);

  program.disableAttrArray("a_pos");
  program.disableAttrArray("a_tex0");

  gles2::CBuffer::unbind(GL_ARRAY_BUFFER);
  gles2::CBuffer::unbind(GL_ELEMENT_ARRAY_BUFFER);
}

void CMesh::draw(CShaderProgram& program, const Indices& indices, bool points)
{
  gles2::CBuffer::bind(m_vbuffer);

  program.enableAttrArray("a_pos");
  program.enableAttrArray("a_tex0");
  program.setAttrBuffer("a_pos", GL_FLOAT, 3, 5 * sizeof(float), 0);
  program.setAttrBuffer(
      "a_tex0", GL_FLOAT, 2, 5 * sizeof(float), 3 * sizeof(float));

  glDrawElements(
      points ? GL_POINTS : GL_TRIANGLE_STRIP,
      indices.size(),
      GL_UNSIGNED_SHORT,
      indices.data());

  program.disableAttrArray("a_pos");
  program.disableAttrArray("a_tex0");

  gles2::CBuffer::unbind(GL_ARRAY_BUFFER);
}

} // namespace gles2
