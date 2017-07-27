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

#include <FreeImage.h>
#include <GLFW/glfw3.h>
#include <array>
#include <cstdlib>
#include <glm/glm.hpp>
#include <iostream>
#include <type_traits>

#include "CBuffer.hpp"
#include "CFrameBuffer.hpp"
#include "CShaderProgram.hpp"
#include "CTexture2D.hpp"

namespace {

constexpr glm::ivec2 c_wnd_size{720, 960};
constexpr char c_wnd_title[] = "Distorted output";
constexpr char c_image_path[] = "bricks.png";

constexpr std::array c_quad_verices = {
  -0.95f, -0.95f, 0.f, 0.0f, 1.0f,
   0.95f, -0.95f, 0.f, 1.0f, 1.0f,
  -0.95f,  0.95f, 0.f, 0.0f, 0.0f,
   0.95f,  0.95f, 0.f, 1.0f, 0.0f,
};

constexpr std::array<uint16_t, 4> c_quad_indices{0, 1, 2, 3};

constexpr char c_vertex_shader_src[] = R"(
  precision highp float;
  attribute vec3 a_pos;
  attribute vec2 a_tex0;
  varying vec2 v_tex0;

  void main()
  {
    gl_Position = vec4(a_pos, 1.0);
    v_tex0 = a_tex0;
  }
)";

constexpr char c_fragment_shader_src[] = R"(
  precision highp float;
  uniform sampler2D u_tex;
  varying vec2 v_tex0;

  void main()
  {
    gl_FragColor = texture2D(u_tex, v_tex0);
  }
)";

constexpr char c_vertex_debug_shader_src[] = R"(
  attribute vec3 a_pos;

  void main()
  {
    gl_PointSize = 2.0;
    gl_Position = vec4(a_pos, 1.0);
  }
)";

constexpr char c_fragment_debug_shader_src[] = R"(
  precision mediump float;
  uniform sampler2D u_tex;
  varying vec2 v_tex0;

  void main()
  {
    gl_FragColor = vec4(0.9, 0.9, 0.0, 0.4);
  }
)";

Gles2::CTexture2D loadImage(const std::string_view& path)
{
  FIBITMAP* bitmap = FreeImage_Load(
    FreeImage_GetFileType(path.data(), 0),
    path.data());
  if (nullptr == bitmap) {
    std::cerr << "Couldn't load image " << path << std::endl;
    throw std::runtime_error("create texture error");
  }

  if (FreeImage_GetBPP(bitmap) != 24) {
    std::cerr << "Unsupported image pixel format" << std::endl;
    throw std::runtime_error("create texture error");
  }

  // Swap red and blue channels, cannot use GL_BGR in OpenGL ES 2
  for (unsigned y = 0; y < FreeImage_GetHeight(bitmap); y++) {
    BYTE* line = FreeImage_GetScanLine(bitmap, y);
    for (unsigned x = 0; x < FreeImage_GetWidth(bitmap); x++) {
      std::swap(line[FI_RGBA_RED], line[FI_RGBA_BLUE]);
      line += FreeImage_GetBPP(bitmap) / 8;
    }
  }

  Gles2::CTexture2D texture(
    {FreeImage_GetWidth(bitmap), FreeImage_GetHeight(bitmap)},
    GL_RGB,
    FreeImage_GetBits(bitmap));

  FreeImage_Unload(bitmap);
  return texture;
}

std::pair<std::vector<float>, std::vector<uint16_t>> generateDistortedQuad()
{
  std::size_t step = 1;
  std::size_t count = 30;

  float dfactor_x = 0.03f;
  float sfactor_x = 5.f;
  float from_x = -0.85f;
  float len_x = 1.7f;

  float dfactor_y = 0.025f;
  float sfactor_y = 16.f;
  float from_y = -0.85f;
  float len_y = 1.7f;

  std::vector<float> dist_vertices;
  for (std::size_t i = 0; i < count; i += step) {
    float x = from_x + (len_x / (count - 1)) * i;
    float u = 0.f + (1.f / (count - 1)) * i;

    float dy = dfactor_y * std::sin(glm::radians(sfactor_y * i));

    for (std::size_t j = 0; j < count; j += step) {
      float y = from_y + (len_y / (count - 1)) * j;
      float v = 1.f - (1.f / (count - 1)) * j;

      float dx = dfactor_x * std::sin(glm::radians(sfactor_x * j));

      dist_vertices.push_back(x + dx);
      dist_vertices.push_back(y + dy);
      dist_vertices.push_back(0.f);

      dist_vertices.push_back(u);
      dist_vertices.push_back(v);
    }
  }

  std::vector<uint16_t> dist_indices;
  for (std::size_t i = 1; i < count; i += step) {
    for (std::size_t j = 0; j < count; j += step) {
      uint16_t x1 = (i - 1) * (count) + j;
      uint16_t x2 = (i - 1) * (count) + j + count;

      if (j == 0 && i != 1) {
        dist_indices.push_back(x1);
      }
      dist_indices.push_back(x1);
      dist_indices.push_back(x2);
    }
    dist_indices.push_back(dist_indices.back());
  }
  return std::make_pair(dist_vertices, dist_indices);
}

void print_glfw_error(int err, const char* msg)
{
  std::cerr << "(" << std::hex << err << ") " << msg << std::endl;
}

} // namespace

int main(int /*argc*/, const char** /*argv*/)
{
  glfwSetErrorCallback(print_glfw_error);
  if (GLFW_TRUE != glfwInit()) {
    std::cerr << "Failed to init GLFW." << std::endl;
    glfwTerminate();
    return EXIT_FAILURE;
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);

  GLFWwindow* window =
    glfwCreateWindow(
      c_wnd_size.x,
      c_wnd_size.y,
      c_wnd_title,
      nullptr,
      nullptr);

  if (nullptr == window) {
    std::cerr << "Failed to create GLFW window." << std::endl;
    glfwTerminate();
    return EXIT_FAILURE;
  }

  glfwMakeContextCurrent(window);
  {

    Gles2::CTexture2D quad_tex = loadImage(c_image_path);
    Gles2::CBuffer quad_vbuffer(GL_ARRAY_BUFFER, c_quad_verices);
    Gles2::CBuffer quad_ibuffer(GL_ELEMENT_ARRAY_BUFFER, c_quad_indices);


    const auto& [dist_vertices, dist_indices] = generateDistortedQuad();
    Gles2::CBuffer dist_quad_vbuffer(GL_ARRAY_BUFFER, dist_vertices);
    Gles2::CBuffer dist_quad_ibuffer(GL_ELEMENT_ARRAY_BUFFER, dist_indices);

    Gles2::CTexture2D fbo_tex(glm::uvec2(1024u), GL_RGB);
    Gles2::CFrameBuffer fbo(fbo_tex);

    Gles2::CShaderProgram dbg_program(c_vertex_debug_shader_src, c_fragment_debug_shader_src);
    Gles2::CShaderProgram program(c_vertex_shader_src, c_fragment_shader_src);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    while (!glfwWindowShouldClose(window)) {
      glfwPollEvents();

      //***********************************************************************
      // Rendering to screen (original scene)
      //***********************************************************************

      Gles2::CShaderProgram::use(program);

      glViewport(0, 0, c_wnd_size.x, c_wnd_size.y / 2);
      glClearColor(0.5f, 0.3f, 0.3f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);

      Gles2::CBuffer::bind(quad_vbuffer);
      Gles2::CBuffer::bind(quad_ibuffer);
      Gles2::CTexture2D::bind(quad_tex);

      program.enableAttrArray("a_pos");
      program.enableAttrArray("a_tex0");

      program.setAttrBuffer("a_pos", GL_FLOAT, 3, 5 * sizeof(float), 0);
      program.setAttrBuffer("a_tex0", GL_FLOAT, 2, 5 * sizeof(float), 3 * sizeof(float));

      glDrawElements(
        GL_TRIANGLE_STRIP,
        c_quad_indices.size(),
        GL_UNSIGNED_SHORT,
        nullptr);

      program.disableAttrArray("a_pos");
      program.disableAttrArray("a_tex0");

      Gles2::CBuffer::unbind(quad_vbuffer);
      Gles2::CBuffer::unbind(quad_ibuffer);
      Gles2::CTexture2D::unbind();

      //***********************************************************************
      // Rendering to texture
      //***********************************************************************

      Gles2::CShaderProgram::use(program);

      Gles2::CFrameBuffer::bind(fbo);

      glViewport(0, 0, fbo_tex.size().x, fbo_tex.size().y);
      glClearColor(0.3f, 0.5f, 0.3f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);

      Gles2::CBuffer::bind(quad_vbuffer);
      Gles2::CBuffer::bind(quad_ibuffer);
      Gles2::CTexture2D::bind(quad_tex);

      program.enableAttrArray("a_pos");
      program.enableAttrArray("a_tex0");

      program.setAttrBuffer("a_pos", GL_FLOAT, 3, 5 * sizeof(float), 0);
      program.setAttrBuffer("a_tex0", GL_FLOAT, 2, 5 * sizeof(float), 3 * sizeof(float));

      glDrawElements(
        GL_TRIANGLE_STRIP,
        c_quad_indices.size(),
        GL_UNSIGNED_SHORT,
        nullptr);

      program.disableAttrArray("a_pos");
      program.disableAttrArray("a_tex0");

      Gles2::CBuffer::unbind(quad_vbuffer);
      Gles2::CBuffer::unbind(quad_ibuffer);
      Gles2::CTexture2D::unbind();

      Gles2::CFrameBuffer::unbind();

      //***********************************************************************
      // Rendering to screen (FBO content)
      //***********************************************************************

      Gles2::CShaderProgram::use(program);

      glViewport(0, c_wnd_size.y / 2, c_wnd_size.x, c_wnd_size.y / 2);

      Gles2::CBuffer::bind(dist_quad_vbuffer);
      Gles2::CBuffer::bind(dist_quad_ibuffer);
      Gles2::CTexture2D::bind(fbo_tex);

      program.enableAttrArray("a_pos");
      program.enableAttrArray("a_tex0");

      program.setAttrBuffer("a_pos", GL_FLOAT, 3, 5 * sizeof(float), 0);
      program.setAttrBuffer("a_tex0", GL_FLOAT, 2, 5 * sizeof(float), 3 * sizeof(float));

      glDrawElements(
        GL_TRIANGLE_STRIP,
        dist_indices.size(),
        GL_UNSIGNED_SHORT,
        nullptr);

      program.disableAttrArray("a_pos");
      program.disableAttrArray("a_tex0");

      Gles2::CBuffer::unbind(dist_quad_vbuffer);
      Gles2::CBuffer::unbind(dist_quad_ibuffer);
      Gles2::CTexture2D::unbind();

      //**************************************

      Gles2::CShaderProgram::use(dbg_program);

      Gles2::CBuffer::bind(dist_quad_vbuffer);
      Gles2::CBuffer::bind(dist_quad_ibuffer);

      dbg_program.enableAttrArray("a_pos");

      dbg_program.setAttrBuffer("a_pos", GL_FLOAT, 3, 5 * sizeof(float), 0);

      glDrawElements(
        GL_POINTS,
        dist_indices.size(),
        GL_UNSIGNED_SHORT,
        nullptr);

      dbg_program.disableAttrArray("a_pos");

      Gles2::CBuffer::unbind(dist_quad_vbuffer);
      Gles2::CBuffer::unbind(dist_quad_ibuffer);

      glfwSwapBuffers(window);
    }
  }

  glfwDestroyWindow(window);
  glfwTerminate();

  return EXIT_SUCCESS;
}
