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

#include "CShaderProgram.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <iostream>
#include <memory>
#include <utility>

namespace gles2 {

CShaderProgram::CShaderProgram(
    const uint8_t* vert_source,
    std::size_t vert_source_len,
    const uint8_t* frag_source,
    std::size_t frag_source_len)
  : m_id(glCreateProgram())
{
  GLuint vert = createShader(GL_VERTEX_SHADER, vert_source, vert_source_len);
  GLuint frag = createShader(GL_FRAGMENT_SHADER, frag_source, frag_source_len);

  glAttachShader(m_id, vert);
  glAttachShader(m_id, frag);
  glLinkProgram(m_id);

  glDeleteShader(vert);
  glDeleteShader(frag);

  GLint success;
  glGetProgramiv(m_id, GL_LINK_STATUS, &success);
  if (!success)
  {
    GLint info_log_len = 0;
    glGetProgramiv(m_id, GL_INFO_LOG_LENGTH, &info_log_len);
    std::unique_ptr<GLchar[]> info_log(new GLchar[info_log_len]);
    glGetProgramInfoLog(m_id, info_log_len, nullptr, info_log.get());
    std::cerr << "Program linking error:\n" << info_log.get() << std::endl;
    glDeleteProgram(m_id);
    m_id = 0;
  }

  if (0 == m_id)
  {
    throw std::runtime_error("couldn't create shader program");
  }
}

CShaderProgram::CShaderProgram(CShaderProgram&& rhs) noexcept
  : m_id(std::move(rhs.m_id))
{
  rhs.m_id = 0u;
}

CShaderProgram& CShaderProgram::operator=(CShaderProgram&& rhs) noexcept
{
  std::swap(m_id, rhs.m_id);
  rhs.m_id = 0u;
  return *this;
}

CShaderProgram::~CShaderProgram()
{
  glDeleteProgram(m_id);
}

GLuint CShaderProgram::createShader(
    GLenum type,
    const uint8_t* source,
    std::size_t source_len)
{
  GLuint shader = glCreateShader(type);
  const GLchar* src = reinterpret_cast<const GLchar*>(source);
  GLint src_len = static_cast<GLint>(source_len);
  glShaderSource(shader, 1, &src, &src_len);
  glCompileShader(shader);

  GLint success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success)
  {
    GLint info_log_len = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_len);
    std::unique_ptr<GLchar[]> info_log(new GLchar[info_log_len]);
    glGetShaderInfoLog(shader, info_log_len, nullptr, info_log.get());
    std::cerr << "Shader compilation error:\n" << info_log.get() << std::endl;
    glDeleteShader(shader);
    shader = 0;
  }

  return shader;
}

void CShaderProgram::setUniform(const std::string_view& name, float value)
{
  if (GLint index = glGetUniformLocation(m_id, name.data()); index > -1)
  {
    glUniform1f(index, value);
  }
}

void CShaderProgram::setUniform(const std::string_view& name, GLint value)
{
  if (GLint index = glGetUniformLocation(m_id, name.data()); index > -1)
  {
    glUniform1i(index, value);
  }
}

void CShaderProgram::setUniform(
    const std::string_view& name,
    const glm::vec2& vec)
{
  if (GLint index = glGetUniformLocation(m_id, name.data()); index > -1)
  {
    glUniform2fv(index, 1, glm::value_ptr(vec));
  }
}

void CShaderProgram::setUniform(
    const std::string_view& name,
    const glm::vec3& vec)
{
  if (GLint index = glGetUniformLocation(m_id, name.data()); index > -1)
  {
    glUniform3fv(index, 1, glm::value_ptr(vec));
  }
}

void CShaderProgram::setUniform(
    const std::string_view& name,
    const glm::vec4& vec)
{
  if (GLint index = glGetUniformLocation(m_id, name.data()); index > -1)
  {
    glUniform4fv(index, 1, glm::value_ptr(vec));
  }
}

void CShaderProgram::setUniform(
    const std::string_view& name,
    const glm::mat3& mat)
{
  if (GLint index = glGetUniformLocation(m_id, name.data()); index > -1)
  {
    glUniformMatrix3fv(index, 1, GL_FALSE, glm::value_ptr(mat));
  }
}

void CShaderProgram::setUniform(
    const std::string_view& name,
    const glm::mat4& mat)
{
  if (GLint index = glGetUniformLocation(m_id, name.data()); index > -1)
  {
    glUniformMatrix4fv(index, 1, GL_FALSE, glm::value_ptr(mat));
  }
}

void CShaderProgram::enableAttrArray(const std::string_view& name)
{
  if (GLint index = glGetAttribLocation(m_id, name.data()); index > -1)
  {
    glEnableVertexAttribArray(static_cast<GLuint>(index));
  }
}

void CShaderProgram::disableAttrArray(const std::string_view& name)
{
  if (GLint index = glGetAttribLocation(m_id, name.data()); index > -1)
  {
    glDisableVertexAttribArray(static_cast<GLuint>(index));
  }
}

void CShaderProgram::setAttrArray(
    const std::string_view& name,
    GLint component_size,
    GLsizei stride,
    const GLfloat* values)
{
  if (GLint index = glGetAttribLocation(m_id, name.data()); index > -1)
  {
    glVertexAttribPointer(
        static_cast<GLuint>(index),
        component_size,
        GL_FLOAT,
        GL_FALSE,
        stride,
        values);
  }
}

void CShaderProgram::setAttrBuffer(
    const std::string_view& name,
    GLenum element_type,
    GLint component_size,
    GLsizei stride,
    GLsizeiptr offset)
{
  if (GLint index = glGetAttribLocation(m_id, name.data()); index > -1)
  {
    glVertexAttribPointer(
        static_cast<GLuint>(index),
        component_size,
        element_type,
        GL_FALSE,
        stride,
        reinterpret_cast<const GLvoid*>(offset));
  }
}

void CShaderProgram::use(const CShaderProgram& prg)
{
  glUseProgram(prg.id());
}

void CShaderProgram::unuse()
{
  glUseProgram(0);
}

} // namespace gles2
