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

#include "CTexture2D.hpp"

#include <FreeImage.h>
#include <iostream>
#include <utility>

namespace gles2 {

CTexture2D::CTexture2D(
    const glm::uvec2 &size,
    GLint format,
    const uint8_t *data)
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

CTexture2D::CTexture2D(CTexture2D &&rhs) noexcept
  : m_id(std::move(rhs.m_id))
  , m_size(std::move(rhs.m_size))
{
  rhs.m_id = 0u;
}

CTexture2D &CTexture2D::operator=(CTexture2D &&rhs) noexcept
{
  std::swap(m_id, rhs.m_id);
  std::swap(m_size, rhs.m_size);
  rhs.m_id = 0;
  return *this;
}

CTexture2D::~CTexture2D()
{
  glDeleteTextures(1, &m_id);
}

void CTexture2D::bind(const CTexture2D &tex, std::size_t unit)
{
  glActiveTexture(GL_TEXTURE0 + unit);
  glBindTexture(GL_TEXTURE_2D, tex.id());
}

void CTexture2D::unbind(std::size_t unit)
{
  glActiveTexture(GL_TEXTURE0 + unit);
  glBindTexture(GL_TEXTURE_2D, 0);
}

CTexture2D CTexture2D::load(const std::string_view &path)
{
  FIBITMAP *bitmap =
      FreeImage_Load(FreeImage_GetFileType(path.data(), 0), path.data());
  if (nullptr == bitmap)
  {
    std::cerr << "Couldn't load image " << path << std::endl;
    throw TextureLoadError("load error");
  }

  int bpp = FreeImage_GetBPP(bitmap);
  if (bpp != 24 && bpp != 32)
  {
    std::cerr << "unsupported image BPP" << std::endl;
    FreeImage_Unload(bitmap);
    throw TextureLoadError("unsupported image bpp");
  }

  // Swap red and blue channels, cannot use GL_BGR in OpenGL ES 2
  for (unsigned y = 0; y < FreeImage_GetHeight(bitmap); y++)
  {
    BYTE *line = FreeImage_GetScanLine(bitmap, y);
    for (unsigned x = 0; x < FreeImage_GetWidth(bitmap); x++)
    {
      std::swap(line[FI_RGBA_RED], line[FI_RGBA_BLUE]);
      line += bpp / 8;
    }
  }

  gles2::CTexture2D texture(
      {FreeImage_GetWidth(bitmap), FreeImage_GetHeight(bitmap)},
      (bpp == 24) ? GL_RGB : GL_RGBA,
      FreeImage_GetBits(bitmap));

  FreeImage_Unload(bitmap);
  return texture;
}

} // namespace gles2
