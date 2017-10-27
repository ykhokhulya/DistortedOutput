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

#include <GLES2/gl2.h>
#include <array>
#include <vector>

namespace gles2 {

class CBuffer
{
public:
  explicit CBuffer(
      GLenum target,
      GLsizeiptr size,
      const GLvoid* data,
      GLenum usage = GL_STATIC_DRAW);

  template <typename T, std::size_t N>
  explicit CBuffer(
      GLenum target,
      const std::array<T, N>& data,
      GLenum usage = GL_STATIC_DRAW);

  template <typename T>
  explicit CBuffer(
      GLenum target,
      const std::vector<T>& data,
      GLenum usage = GL_STATIC_DRAW);

  CBuffer(CBuffer&& rhs) noexcept;
  CBuffer& operator=(CBuffer&& rhs) noexcept;
  CBuffer(const CBuffer&) = delete;
  CBuffer& operator=(CBuffer&) = delete;
  ~CBuffer();

  GLuint id() const { return m_id; }
  GLuint target() const { return m_target; }

  void update(GLintptr offset, GLsizeiptr size, const GLvoid* data);

  template <typename T>
  void update(std::size_t index, const T& data);

  static void bind(const CBuffer& buf);
  static void unbind(GLenum target);

private:
  GLuint m_id;
  GLenum m_target;
};

template <typename T, std::size_t N>
CBuffer::CBuffer(GLenum target, const std::array<T, N>& data, GLenum usage)
  : CBuffer(target, sizeof(T) * N, data.data(), usage)
{
}

template <typename T>
CBuffer::CBuffer(GLenum target, const std::vector<T>& data, GLenum usage)
  : CBuffer(target, sizeof(T) * data.size(), data.data(), usage)
{
}

template <typename T>
void CBuffer::update(std::size_t index, const T& data)
{
  update(sizeof(T) * index, sizeof(T), &data);
}

} // namespace gles2
