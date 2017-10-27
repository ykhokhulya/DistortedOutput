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

#pragma once

#include <cstddef>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vector>

#include "CBuffer.hpp"
#include "CShaderProgram.hpp"

namespace gles2 {

class CMesh
{
public:
  struct Vertex
  {
    glm::vec3 position;
    glm::vec2 text0;
  };

  using Vertices = std::vector<Vertex>;
  using Indices = std::vector<uint16_t>;

public:
  explicit CMesh(Vertices vertices, Indices indices);

  const Vertices& getVertices() const { return m_vertices; }
  const Indices& getIndices() const { return m_indices; }

  const gles2::CBuffer& getIBuffer() const { return m_ibuffer; }
  const gles2::CBuffer& getVBuffer() const { return m_vbuffer; }

  void draw(CShaderProgram& prg, bool points = false);
  void draw(CShaderProgram& prg, const Indices& indices, bool points = false);

private:
  Vertices m_vertices;
  Indices m_indices;
  gles2::CBuffer m_vbuffer;
  gles2::CBuffer m_ibuffer;
};

} // namespace gles2
