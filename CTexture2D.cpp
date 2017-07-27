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
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*******************************************************************************/

#include "CTexture2D.hpp"

#include <utility>

namespace Gles2 {

CTexture2D::CTexture2D(
  const glm::uvec2& size,
  GLint format,
  const uint8_t* data)
  : m_size(size)
{
  glGenTextures(1, &m_id);

  glBindTexture(GL_TEXTURE_2D, m_id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(
    GL_TEXTURE_2D,
    0,
    format,
    size.x,
    size.y,
    0,
    format,
    GL_UNSIGNED_BYTE,
    data);
  glBindTexture(GL_TEXTURE_2D, 0);
}

CTexture2D::CTexture2D(CTexture2D&& rhs) noexcept
  : m_id(std::move(rhs.m_id))
  , m_size(std::move(rhs.m_size))
{
  rhs.m_id = 0u;
}

CTexture2D::~CTexture2D()
{
  glDeleteTextures(1, &m_id);
}

void CTexture2D::bind(const CTexture2D& tex)
{
  glBindTexture(GL_TEXTURE_2D, tex.id());
}

void CTexture2D::unbind()
{
  glBindTexture(GL_TEXTURE_2D, 0);
}

} // namespace Gles2
