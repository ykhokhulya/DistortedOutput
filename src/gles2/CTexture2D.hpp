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
#include <glm/vec2.hpp>
#include <stdexcept>
#include <string_view>

namespace gles2 {

struct TextureLoadError : std::runtime_error
{
  using std::runtime_error::runtime_error;
};

class CTexture2D
{
public:
  explicit CTexture2D(
      const glm::uvec2 &size,
      GLint format,
      const uint8_t *data = nullptr);
  CTexture2D(CTexture2D &&rhs) noexcept;
  CTexture2D &operator=(CTexture2D &&rhs) noexcept;
  CTexture2D(const CTexture2D &) = delete;
  CTexture2D &operator=(CTexture2D &) = delete;
  ~CTexture2D();

  GLuint id() const { return m_id; }
  const glm::uvec2 &size() const { return m_size; }

public:
  static CTexture2D load(const std::string_view &path);

  static void bind(const CTexture2D &tex, std::size_t unit = 0);
  static void unbind(std::size_t unit = 0);

private:
  GLuint m_id;
  glm::uvec2 m_size;
};

} // namespace gles2
