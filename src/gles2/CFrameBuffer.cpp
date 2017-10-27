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

#include "CFrameBuffer.hpp"

#include <iostream>
#include <utility>

#include "CTexture2D.hpp"

namespace gles2 {

CFrameBuffer::CFrameBuffer(const CTexture2D& color)
  : m_id(0)
{
  glGenFramebuffers(1, &m_id);
  glBindFramebuffer(GL_FRAMEBUFFER, m_id);

  glFramebufferTexture2D(
      GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color.id(), 0);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
  {
    std::cerr << "Framebuffer creation failed." << std::endl;
    glDeleteFramebuffers(1, &m_id);
    m_id = 0;
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  if (0 == m_id)
  {
    throw std::runtime_error("couldn't create frame buffer");
  }
}

CFrameBuffer::CFrameBuffer(CFrameBuffer&& rhs) noexcept
  : m_id(std::move(rhs.m_id))
{
  rhs.m_id = 0u;
}

CFrameBuffer& CFrameBuffer::operator=(CFrameBuffer&& rhs) noexcept
{
  std::swap(m_id, rhs.m_id);
  rhs.m_id = 0u;
  return *this;
}

CFrameBuffer::~CFrameBuffer()
{
  glDeleteFramebuffers(1, &m_id);
}

void CFrameBuffer::bind(const CFrameBuffer& fb)
{
  glBindFramebuffer(GL_FRAMEBUFFER, fb.id());
}

void CFrameBuffer::unbind()
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

} // namespace gles2
