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

#pragma once

#include <GLES2/gl2.h>
#include <glm/fwd.hpp>
#include <string_view>

namespace Gles2 {

class CShaderProgram
{
public:
  explicit CShaderProgram(
    const uint8_t* vert_source,
    std::size_t vert_source_len,
    const uint8_t* frag_source,
    std::size_t frag_source_len);

  explicit CShaderProgram(
    const std::string_view& vert_source,
    const std::string_view& frag_source);

  CShaderProgram(CShaderProgram&& rhs) noexcept;
  CShaderProgram(const CShaderProgram&) = delete;
  ~CShaderProgram();

  GLuint id() const { return m_id; }

  void setUniform(const std::string_view& name, float value);
  void setUniform(const std::string_view& name, GLint value);
  void setUniform(const std::string_view& name, const glm::vec2& vec);
  void setUniform(const std::string_view& name, const glm::vec3& vec);
  void setUniform(const std::string_view& name, const glm::vec4& vec);
  void setUniform(const std::string_view& name, const glm::mat3& mat);
  void setUniform(const std::string_view& name, const glm::mat4& mat);

  void enableAttrArray(const std::string_view& name);
  void disableAttrArray(const std::string_view& name);

  void setAttrArray(
      const std::string_view& name,
      GLint component_size,
      GLsizei stride,
      const GLfloat* values);

  void setAttrBuffer(
      const std::string_view& name,
      GLenum element_type,
      GLint component_size,
      GLsizei stride,
      GLsizeiptr offset);

  static void use(const CShaderProgram& prg);
  static void unuse();

private:
  static GLuint createShader(
    GLenum type,
    const uint8_t* source,
    std::size_t source_len);

  GLuint m_id;
};

inline CShaderProgram::CShaderProgram(
  const std::string_view& vert_source,
  const std::string_view& frag_source)
  : CShaderProgram(
    reinterpret_cast<const uint8_t*>(vert_source.data()),
    vert_source.size(),
    reinterpret_cast<const uint8_t*>(frag_source.data()),
    frag_source.size())
{
}

} // namespace Gles2
