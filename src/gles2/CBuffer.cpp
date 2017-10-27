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

#include "CBuffer.hpp"

#include <utility>

namespace gles2 {

CBuffer::CBuffer(
    GLenum target,
    GLsizeiptr size,
    const GLvoid* data,
    GLenum usage)
  : m_id(0)
  , m_target(target)
{
  glGenBuffers(1, &m_id);
  glBindBuffer(m_target, m_id);
  glBufferData(m_target, size, data, usage);
  glBindBuffer(m_target, 0);
}

CBuffer::CBuffer(CBuffer&& rhs) noexcept
  : m_id(std::move(rhs.m_id))
  , m_target(std::move(rhs.m_target))
{
  rhs.m_id = 0u;
}

CBuffer& CBuffer::operator=(CBuffer&& rhs) noexcept
{
  std::swap(m_id, rhs.m_id);
  std::swap(m_target, rhs.m_target);
  rhs.m_id = 0;
  return *this;
}

CBuffer::~CBuffer()
{
  glDeleteBuffers(1, &m_id);
}

void CBuffer::update(GLintptr offset, GLsizeiptr size, const GLvoid* data)
{
  glBufferSubData(m_target, offset, size, data);
}

void CBuffer::bind(const CBuffer& buf)
{
  glBindBuffer(buf.target(), buf.id());
}

void CBuffer::unbind(GLenum target)
{
  glBindBuffer(target, 0);
}

} // namespace gles2
