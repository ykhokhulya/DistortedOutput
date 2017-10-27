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

#include <GLFW/glfw3.h>
#include <array>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtx/transform.hpp>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>

#include "gles2/CBuffer.hpp"
#include "gles2/CFrameBuffer.hpp"
#include "gles2/CMesh.hpp"
#include "gles2/CShaderProgram.hpp"
#include "gles2/CTexture2D.hpp"

namespace {

constexpr glm::ivec2 c_wnd_size{640, 480};
constexpr char c_wnd_title[] = "Distorted output";

constexpr std::size_t c_num_points = 30;
constexpr float c_shift_len = 0.01f;
constexpr float c_zoom_factor = 1.05f;

constexpr std::array c_image_paths = {
    "bricks.png",
};

constexpr char c_img_vshader_src[] = R"(
  precision highp float;
  attribute vec3 a_pos;
  attribute vec2 a_tex0;
  uniform mat4 u_mvp;
  varying vec2 v_tex0;

  void main() {
    gl_Position = u_mvp * vec4(a_pos, 1.0);
    v_tex0 = a_tex0;
  }
)";

constexpr char c_img_fshader_src[] = R"(
  precision highp float;
  uniform sampler2D u_tex;
  varying vec2 v_tex0;

  void main() {
    gl_FragColor = texture2D(u_tex, v_tex0);
  }
)";

constexpr char c_dbg_vshader_src[] = R"(
  attribute vec3 a_pos;
  attribute vec2 a_tex0;
  uniform mat4 u_mvp;
  uniform float u_pnt_sz;

  void main() {
    gl_PointSize = u_pnt_sz;
    gl_Position = u_mvp * vec4(a_pos, 1.0);
  }
)";

constexpr char c_dbg_fshader_src[] = R"(
  precision mediump float;
  uniform sampler2D u_tex;
  uniform vec4 u_col;
  varying vec2 v_tex0;

  void main() {
    gl_FragColor = u_col;
  }
)";

/// Key points order:
///
/// P03 -- P13 -- P23 -- P33
///  |      |      |      |
/// P02 -- P12 -- P22 -- P32
///  |      |      |      |
/// P01 -- P11 -- P21 -- P31
///  |      |      |      |
/// P00 -- P10 -- P20 -- P30

constexpr std::array c_key_points = {
    /*P00*/ glm::vec2(-1.0, -1.0),
    /*P01*/ glm::vec2(-1.0, -0.33333),
    /*P02*/ glm::vec2(-1.0, 0.33333),
    /*P03*/ glm::vec2(-1.0, 1.0),

    /*P10*/ glm::vec2(-0.33333, -1.0),
    /*P11*/ glm::vec2(-0.33333, -0.33333),
    /*P12*/ glm::vec2(-0.33333, 0.33333),
    /*P13*/ glm::vec2(-0.33333, 1.0),

    /*P20*/ glm::vec2(0.33333, -1.0),
    /*P21*/ glm::vec2(0.33333, -0.33333),
    /*P22*/ glm::vec2(0.33333, 0.33333),
    /*P23*/ glm::vec2(0.33333, 1.0),

    /*P30*/ glm::vec2(1.0, -1.0),
    /*P31*/ glm::vec2(1.0, -0.33333),
    /*P32*/ glm::vec2(1.0, 0.33333),
    /*P33*/ glm::vec2(1.0, 1.0),
};

constexpr std::pair<int, glm::vec2> c_shift_keys[] = {
    {GLFW_KEY_KP_4, glm::ivec2(-1, 0)},
    {GLFW_KEY_KP_6, glm::ivec2(1, 0)},
    {GLFW_KEY_KP_8, glm::ivec2(0, 1)},
    {GLFW_KEY_KP_2, glm::ivec2(0, -1)},
};

constexpr std::pair<int, glm::ivec2> c_choose_point_keys[] = {
    {GLFW_KEY_A, glm::ivec2(-1, 0)},
    {GLFW_KEY_D, glm::ivec2(1, 0)},
    {GLFW_KEY_W, glm::ivec2(0, 1)},
    {GLFW_KEY_S, glm::ivec2(0, -1)},
};

constexpr std::pair<int, int> c_choose_image_keys[] = {
    {GLFW_KEY_PAGE_UP, 1},
    {GLFW_KEY_PAGE_DOWN, -1},
};

constexpr std::pair<int, float> c_choose_zoom_keys[] = {
    {GLFW_KEY_KP_ADD, c_zoom_factor},
    {GLFW_KEY_KP_SUBTRACT, 1 / c_zoom_factor},
};

constexpr int c_toggle_image_key = GLFW_KEY_HOME;
constexpr int c_toggle_points_key = GLFW_KEY_END;
constexpr int c_save_kps_key = GLFW_KEY_F12;
constexpr int c_close_wnd_key = GLFW_KEY_ESCAPE;
constexpr int c_reset_points_key = GLFW_KEY_R;
constexpr int c_reload_points_key = GLFW_KEY_L;

int g_pnt_index;
int g_image_index;
glm::vec2 g_shift;
float g_img_zoom = 1.f;

bool g_enable_image;
bool g_enable_points;
bool g_request_to_update_mesh;
bool g_request_to_save_kps;
bool g_request_to_stop_wnd;
bool g_request_to_reset_kps;
bool g_request_to_reload_kps;

// gles2::CMesh generatePattern(std::size_t count)
//{
//  gles2::CMesh::Vertices dist_vertices;
//  for (std::size_t i = 0; i < count; ++i) {
//    float x = -1.f + 2.f * i / (count - 1);
//    float u = static_cast<float>(i) / (count - 1);

//    for (std::size_t j = 0; j < count; ++j) {
//      float y = -1.f + 2.f * j / (count - 1);
//      float v = 1.f - static_cast<float>(j) / (count - 1);

//      dist_vertices.push_back({glm::vec3(x, y, 0.f), glm::vec2(u, v)});
//    }
//  }

//  gles2::CMesh::Indices dist_indices;
//  for (std::size_t i = 1; i < count; ++i) {
//    for (std::size_t j = 0; j < count; ++j) {
//      uint16_t x1 = (i - 1) * (count) + j;
//      uint16_t x2 = (i - 1) * (count) + j + count;

//      if (j == 0 && i != 1) {
//        dist_indices.push_back(x1);
//      }
//      dist_indices.push_back(x1);
//      dist_indices.push_back(x2);
//    }
//    dist_indices.push_back(dist_indices.back());
//  }

//  return gles2::CMesh(std::move(dist_vertices), std::move(dist_indices));
//}

gles2::CMesh generateDistortionMesh(
    std::size_t count,
    const std::array<glm::vec2, 16>& kps)
{
  struct Math
  {
    static glm::vec2 bezier(
        const glm::vec2& p0,
        const glm::vec2& p1,
        const glm::vec2& p2,
        const glm::vec2& p3,
        float u)
    {
      return std::pow(u, 3.f) * p3 + 3 * std::pow(u, 2.f) * (1.f - u) * p2 +
             3 * u * std::pow(1.f - u, 2.f) * p1 + std::pow(1.f - u, 3.f) * p0;
    }

    static std::pair<std::vector<glm::vec2>, std::vector<glm::vec2>>
    bezierControlPoints(std::vector<glm::vec2> knots)
    {
      // https://www.particleincell.com/2012/bezier-splines/
      std::size_t num = knots.size() - 1;

      std::vector<glm::vec2> p1(num);
      std::vector<glm::vec2> p2(num);

      /*rhs vector*/
      std::vector<float> a(num);
      std::vector<float> b(num);
      std::vector<float> c(num);
      std::vector<glm::vec2> r(num);

      /*left most segment*/
      a[0] = 0;
      b[0] = 2;
      c[0] = 1;
      r[0] = knots[0] + knots[1] * 2.f;

      /*internal segments*/
      for (int i = 1; i < num - 1; i++)
      {
        a[i] = 1;
        b[i] = 4;
        c[i] = 1;
        r[i] = knots[i] * 4.f + knots[i + 1] * 2.f;
      }

      /*right segment*/
      a[num - 1] = 2;
      b[num - 1] = 7;
      c[num - 1] = 0;
      r[num - 1] = knots[num - 1] * 8.f + knots[num];

      /*solves Ax=b with the Thomas algorithm (from Wikipedia)*/
      for (int i = 1; i < num; ++i)
      {
        float m = a[i] / b[i - 1];
        b[i] = b[i] - c[i - 1] * m;
        r[i] = r[i] - r[i - 1] * m;
      }

      p1[num - 1] = r[num - 1] / b[num - 1];
      for (int i = num - 2; i >= 0; --i)
      {
        p1[i] = (r[i] - p1[i + 1] * c[i]) / b[i];
      }

      /*we have p1, now compute p2*/
      for (int i = 0; i < num - 1; ++i)
      {
        p2[i] = knots[i + 1] * 2.f - p1[i + 1];
      }

      p2[num - 1] = (knots[num] + p1[num - 1]) * 0.5f;

      return std::make_pair(p1, p2);
    }
  };

  gles2::CMesh::Vertices dist_vertices;
  const auto & [ p1y0, p2y0 ] =
      Math::bezierControlPoints({kps[0], kps[4], kps[8], kps[12]});
  const auto & [ p1y1, p2y1 ] =
      Math::bezierControlPoints({kps[1], kps[5], kps[9], kps[13]});
  const auto & [ p1y2, p2y2 ] =
      Math::bezierControlPoints({kps[2], kps[6], kps[10], kps[14]});
  const auto & [ p1y3, p2y3 ] =
      Math::bezierControlPoints({kps[3], kps[7], kps[11], kps[15]});

  for (std::size_t i = 0; i < 3; ++i)
  {
    for (std::size_t ii = 0; ii < count / 3; ++ii)
    {
      float px = static_cast<float>(ii) / (count / 3 - 1);
      glm::vec2 p0y = Math::bezier(
          kps[4 * i + 0], p1y0[i], p2y0[i], kps[4 * (i + 1) + 0], px);
      glm::vec2 p1y = Math::bezier(
          kps[4 * i + 1], p1y1[i], p2y1[i], kps[4 * (i + 1) + 1], px);
      glm::vec2 p2y = Math::bezier(
          kps[4 * i + 2], p1y2[i], p2y2[i], kps[4 * (i + 1) + 2], px);
      glm::vec2 p3y = Math::bezier(
          kps[4 * i + 3], p1y3[i], p2y3[i], kps[4 * (i + 1) + 3], px);

      std::vector<glm::vec2> knots = {p0y, p1y, p2y, p3y};
      const auto & [ p1x0, p2x0 ] = Math::bezierControlPoints(knots);

      for (std::size_t j = 0; j < 3; ++j)
      {
        for (std::size_t jj = 0; jj < count / 3; ++jj)
        {
          float py = static_cast<float>(jj) / (count / 3 - 1);
          glm::vec2 p =
              Math::bezier(knots[j], p1x0[j], p2x0[j], knots[j + 1], py);

          dist_vertices.push_back(
              {glm::vec3(std::move(p), 0.f),
               glm::vec2(i / 3.f + 1 / 3.f * px, j / 3.f + 1 / 3.f * py)});
        }
      }
    }
  }

  //  gles2::CMesh::Vertices dist_vertices;
  //  for (std::size_t i = 0; i < count; ++i) {
  //    float px = static_cast<float>(i) / (count - 1);

  //    glm::vec2 p0y = Math::bezier(kps[0], kps[4], kps[ 8], kps[12], px);
  //    glm::vec2 p1y = Math::bezier(kps[1], kps[5], kps[ 9], kps[13], px);
  //    glm::vec2 p2y = Math::bezier(kps[2], kps[6], kps[10], kps[14], px);
  //    glm::vec2 p3y = Math::bezier(kps[3], kps[7], kps[11], kps[15], px);

  //    for (std::size_t j = 0; j < count; ++j) {
  //      float py = static_cast<float>(j) / (count - 1);
  //      glm::vec2 p = Math::bezier(p0y, p1y, p2y, p3y, py);

  //      dist_vertices.push_back(
  //        {glm::vec3(std::move(p), 0.f), glm::vec2(px, py)});
  //    }
  //  }

  gles2::CMesh::Indices dist_indices;
  for (std::size_t i = 1; i < count; ++i)
  {
    for (std::size_t j = 0; j < count; ++j)
    {
      uint16_t x1 = (i - 1) * (count) + j;
      uint16_t x2 = (i - 1) * (count) + j + count;

      if (j == 0 && i != 1)
      {
        dist_indices.push_back(x1);
      }
      dist_indices.push_back(x1);
      dist_indices.push_back(x2);
    }
    dist_indices.push_back(dist_indices.back());
  }

  return gles2::CMesh(std::move(dist_vertices), std::move(dist_indices));
}

gles2::CMesh generateKeyPointsMesh(std::array<glm::vec2, 16> kps_mesh)
{
  gles2::CMesh::Vertices dist_vertices;
  for (std::size_t i = 0; i < kps_mesh.size(); ++i)
  {
    dist_vertices.push_back(
        {glm::vec3(std::move(kps_mesh[i]), 0.f), glm::vec2(0.f)});
  }

  gles2::CMesh::Indices dist_indices = {
      0, 4,  1, 5,  2,  6, 3, 7,  7, 4,  4,  8,  5,  9,
      6, 10, 7, 11, 11, 8, 8, 12, 9, 13, 10, 14, 11, 15,
  };
  return gles2::CMesh(std::move(dist_vertices), std::move(dist_indices));
}

std::string getCurrentDateTime()
{
  auto now = std::chrono::system_clock::now();
  auto in_time_t = std::chrono::system_clock::to_time_t(now);

  std::stringstream ss;
  ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d_%X");
  return ss.str();
}

void storeKeyPoints(
    const std::array<glm::vec2, 16>& kps,
    const std::string_view& filename)
{
  if (std::ofstream file(filename.data()); file)
  {
    for (auto&& p : kps)
    {
      file << p.x << " " << p.y << "\n";
    }
    std::cout << "Saved to " << std::quoted(filename.data()) << std::endl;
  }
}

std::array<glm::vec2, 16> loadKeyPoints(const std::string_view& filename)
{
  std::array<glm::vec2, 16> kps;
  if (std::ifstream file(filename.data()); file)
  {
    for (auto& p : kps)
    {
      file >> p.x >> p.y;
    }
  }
  return kps;
}

void key_callback(GLFWwindow*, int key, int, int action, int)
{
  if (g_enable_points)
  {
    for (auto[k, v] : c_shift_keys)
    {
      if (key == k && action == GLFW_PRESS)
      {
        g_shift = v * c_shift_len;
        std::cout << "Move point: " << g_shift << std::endl;
        g_request_to_update_mesh = true;
      }
    }
    for (auto[k, v] : c_choose_point_keys)
    {
      if (key == k && action == GLFW_PRESS)
      {
        g_pnt_index += v.x * 4 + v.y;
        g_pnt_index = glm::mod<float>(g_pnt_index, c_key_points.size());
        std::cout << "Select point: " << g_pnt_index << std::endl;
      }
    }
  }
  if (g_enable_image)
  {
    for (auto[k, v] : c_choose_image_keys)
    {
      if (key == k && action == GLFW_PRESS)
      {
        g_image_index += v;
        g_image_index = glm::mod<float>(g_image_index, c_image_paths.size());
        std::cout << "Select image: " << g_image_index << std::endl;
      }
    }
  }
  if (g_enable_image || g_enable_points)
  {
    for (auto[k, v] : c_choose_zoom_keys)
    {
      if (key == k && action == GLFW_PRESS)
      {
        g_img_zoom *= v;
        std::cout << "Change zoom: " << g_img_zoom << std::endl;
      }
    }
  }
  if (key == c_toggle_image_key && action == GLFW_PRESS)
  {
    g_enable_image = !g_enable_image;
    std::cout << "Enable image: " << g_enable_image << std::endl;
  }
  if (key == c_toggle_points_key && action == GLFW_PRESS)
  {
    g_enable_points = !g_enable_points;
    std::cout << "Enable points: " << g_enable_points << std::endl;
  }

  if (key == c_save_kps_key && action == GLFW_PRESS)
  {
    g_request_to_save_kps = true;
    std::cout << "Request to save points." << std::endl;
  }
  if (key == c_reset_points_key && action == GLFW_PRESS)
  {
    g_request_to_reset_kps = true;
    std::cout << "Request to reset kps." << std::endl;
  }
  if (key == c_reload_points_key && action == GLFW_PRESS)
  {
    g_request_to_reload_kps = true;
    std::cout << "Request to reload kps." << std::endl;
  }

  if (key == c_close_wnd_key && action == GLFW_PRESS)
  {
    g_request_to_stop_wnd = true;
    std::cout << "Request to close window." << std::endl;
  }
}

} // namespace

int main(int argc, const char** argv)
{
  glfwSetErrorCallback([](int err, const char* msg) {
    std::cerr << "(" << std::hex << err << ") " << msg << std::endl;
  });

  if (GLFW_TRUE != glfwInit())
  {
    std::cerr << "Failed to init GLFW." << std::endl;
    glfwTerminate();
    return EXIT_FAILURE;
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);

  GLFWwindow* window = glfwCreateWindow(
      c_wnd_size.x, c_wnd_size.y, c_wnd_title, nullptr, nullptr);

  if (nullptr == window)
  {
    std::cerr << "Failed to create GLFW window." << std::endl;
    glfwTerminate();
    return EXIT_FAILURE;
  }

  std::array key_points = (argc == 2) ? loadKeyPoints(argv[1]) : c_key_points;

  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
  glfwSetKeyCallback(window, key_callback);

  glfwMakeContextCurrent(window);
  {
    std::vector<gles2::CTexture2D> images;
    for (auto&& path : c_image_paths)
    {
      images.push_back(gles2::CTexture2D::load(path));
    }

    gles2::CMesh dist_mesh = generateDistortionMesh(c_num_points, key_points);
    gles2::CMesh kps_mesh = generateKeyPointsMesh(key_points);

    gles2::CShaderProgram pts_program(c_dbg_vshader_src, c_dbg_fshader_src);
    gles2::CShaderProgram img_program(c_img_vshader_src, c_img_fshader_src);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    while (!glfwWindowShouldClose(window))
    {
      glfwPollEvents();

      if (g_request_to_stop_wnd)
      {
        break;
      }
      if (g_request_to_reset_kps)
      {
        key_points = c_key_points;
        g_request_to_reset_kps = false;
        g_request_to_update_mesh = true;
      }
      if (g_request_to_reload_kps)
      {
        if (argc == 2)
        {
          key_points = loadKeyPoints(argv[1]);
        }
        g_request_to_reload_kps = false;
        g_request_to_update_mesh = true;
      }
      if (g_request_to_update_mesh)
      {
        key_points[g_pnt_index] += g_shift;
        dist_mesh = generateDistortionMesh(c_num_points, key_points);
        kps_mesh = generateKeyPointsMesh(key_points);
        g_request_to_update_mesh = false;
      }
      if (g_request_to_save_kps)
      {
        std::string filename = getCurrentDateTime() + ".hcd";
        storeKeyPoints(key_points, filename);
        g_request_to_save_kps = false;
      }

      glClear(GL_COLOR_BUFFER_BIT);

      glm::ivec2 wnd_size;
      glfwGetWindowSize(window, &wnd_size.x, &wnd_size.y);
      glViewport(0, 0, wnd_size.x, wnd_size.y);

      if (g_enable_image)
      {
        gles2::CShaderProgram::use(img_program);
        img_program.setUniform("u_mvp", glm::scale(glm::vec3(g_img_zoom)));
        gles2::CTexture2D::bind(images[g_image_index]);
        dist_mesh.draw(img_program);
      }

      if (g_enable_points)
      {
        gles2::CShaderProgram::use(pts_program);
        pts_program.setUniform("u_mvp", glm::scale(glm::vec3(g_img_zoom)));

        pts_program.setUniform("u_pnt_sz", 20.f);
        pts_program.setUniform("u_col", glm::vec4(1.f, 1.f, 0.f, .7f));
        kps_mesh.draw(pts_program, {static_cast<uint16_t>(g_pnt_index)}, true);

        pts_program.setUniform("u_pnt_sz", 10.f);
        pts_program.setUniform("u_col", glm::vec4(1.f, 0.f, 1.f, .7f));
        kps_mesh.draw(pts_program, true);

        pts_program.setUniform("u_pnt_sz", 2.f);
        pts_program.setUniform("u_col", glm::vec4(0.f, 1.f, 1.f, .7f));
        dist_mesh.draw(pts_program, true);
      }

      glfwSwapBuffers(window);
    }
  }

  glfwDestroyWindow(window);
  glfwTerminate();

  return EXIT_SUCCESS;
}
