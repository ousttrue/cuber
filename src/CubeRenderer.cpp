#include "CubeRenderer.h"
#include <DirectXMath.h>
#include <GL/glew.h>
#include <iostream>

const int CUBE_INDEX_COUNT = 36;

namespace cuber {

static const char *vertex_shader_text = R"(
uniform mat4 VP;
in vec3 vPos;
in vec2 vBarycentric;
in vec4 iRow0;
in vec4 iRow1;
in vec4 iRow2;
in vec4 iRow3;
out vec2 barycentric;

mat4 transform(vec4 r0, vec4 r1, vec4 r2, vec4 r3)
{
  return mat4(
    r0.x, r0.y, r0.z, r0.w,
    r1.x, r1.y, r1.z, r1.w,
    r2.x, r2.y, r2.z, r2.w,
    r3.x, r3.y, r3.z, r3.w
  );
}

void main()
{
    gl_Position = VP * transform(iRow0, iRow1, iRow2, iRow3) * vec4(vPos, 1.0);
    barycentric = vBarycentric;
}
)";

struct XY {
  float x;
  float y;
};

struct XYZ {
  float x;
  float y;
  float z;
};

struct XYZW {
  float x;
  float y;
  float z;
  float w;
};

struct RGBA {
  float r;
  float g;
  float b;
  float a;
};

struct Instance {
  XYZW row0;
  XYZW row1;
  XYZW row2;
  XYZW row3;
};
struct Vertex {
  XYZ position;
  XY barycentric;
};

static const char *fragment_shader_text = R"(
in vec2 barycentric;
out vec4 FragColor;

// float grid (vec2 vBC, float width, float feather) {
//   float w1 = width - feather * 0.5;
//   vec3 bary = vec3(vBC.x, vBC.y, 1.0 - vBC.x - vBC.y);
//   vec3 d = fwidth(bary);
//   vec3 a3 = smoothstep(d * w1, d * (w1 + feather), bary);
//   return min(min(a3.x, a3.y), a3.z);
// }

// https://github.com/rreusser/glsl-solid-wireframe
float grid (vec2 vBC, float width) {
  vec3 bary = vec3(vBC.x, vBC.y, 1.0 - vBC.x - vBC.y);
  vec3 d = fwidth(bary);
  vec3 a3 = smoothstep(d * (width - 0.5), d * (width + 0.5), bary);
  return min(a3.x, a3.y);
}

void main()
{
    FragColor = vec4(vec3(grid(barycentric, 1.0)), 1);
}
)";

const float s = 0.5f;
XYZ positions[8] = {
    {-s, -s, +s}, //
    {-s, +s, +s}, //
    {+s, +s, +s}, //
    {+s, -s, +s}, //
    {-s, -s, -s}, //
    {-s, +s, -s}, //
    {+s, +s, -s}, //
    {+s, -s, -s}, //
};

//   5+-+6
//   / /|
// 1+-+2|
//  |4+-+7
//  |/ /
// 0+-+3
//
//   Y
//   A
//   +-> X
//  /
// L
//
struct Face {
  int indices[4];
  RGBA color;
};

// CCW
Face cube_faces[6] = {
    {
        .indices = {0, 3, 2, 1},
        .color = {1, 0, 0, 1},
    },
    {
        .indices = {3, 7, 6, 2},
        .color = {0, 1, 0, 1},
    },
    {
        .indices = {7, 4, 5, 6},
        .color = {0, 0, 1, 1},
    },
    {
        .indices = {4, 0, 1, 5},
        .color = {0, 1, 1, 1},
    },
    {
        .indices = {1, 2, 6, 5},
        .color = {1, 0, 1, 1},
    },
    {
        .indices = {3, 0, 4, 7},
        .color = {1, 1, 0, 1},
    },
};

struct Builder {
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
  void Quad(const XYZ &p0, const XYZ &p1, const XYZ &p2, const XYZ &p3,
            const RGBA &color) {
    // 01   00
    //  3+-+2
    //   | |
    //  0+-+1
    // 00   10
    Vertex v0{
        .position = p0,
        .barycentric = {1, 0},
    };
    Vertex v1{
        .position = p1,
        .barycentric = {0, 0},
    };
    Vertex v2{
        .position = p2,
        .barycentric = {0, 1},
    };
    Vertex v3{
        .position = p3,
        .barycentric = {0, 0},
    };
    auto index = vertices.size();
    vertices.push_back(v0);
    vertices.push_back(v1);
    vertices.push_back(v2);
    vertices.push_back(v3);
    // 0, 1, 2
    indices.push_back(index);
    indices.push_back(index + 1);
    indices.push_back(index + 2);
    // 2, 3, 0
    indices.push_back(index + 2);
    indices.push_back(index + 3);
    indices.push_back(index);
  }
};

static uint32_t
get_location(const std::shared_ptr<cuber::ShaderProgram> &shader,
             const char *name) {
  auto location = shader->AttributeLocation(name);
  if (!location) {
    throw std::runtime_error("glGetUniformLocation");
  }
  return *location;
}

CubeRenderer::CubeRenderer() {

  // auto glsl_version = "#version 150";
  auto glsl_version = "#version 310 es\nprecision highp float;";

  std::string_view vs[] = {
      glsl_version,
      "\n",
      vertex_shader_text,
  };
  std::string_view fs[] = {
      glsl_version,
      "\n",
      fragment_shader_text,
  };
  shader_ = cuber::ShaderProgram::Create(
      [](auto msg) { std::cout << msg << std::endl; }, vs, fs);
  if (!shader_) {
    throw std::runtime_error("cuber::ShaderProgram::Create");
  }

  auto vpos_location = get_location(shader_, "vPos");
  auto vbarycentric_location = get_location(shader_, "vBarycentric");
  // auto ipos_location = get_location(shader_, "iPos");
  // auto irot_location = get_location(shader_, "iRot");

  Builder builder;
  for (auto face : cube_faces) {
    builder.Quad(positions[face.indices[0]], positions[face.indices[1]],
                 positions[face.indices[2]], positions[face.indices[3]],
                 face.color);
  }

  auto vbo = cuber::Vbo::Create(sizeof(Vertex) * builder.vertices.size(),
                                builder.vertices.data());
  if (!vbo) {
    throw std::runtime_error("cuber::Vbo::Create");
  }

  instance_vbo_ = cuber::Vbo::Create(sizeof(float) * 16 * 65535);
  if (!instance_vbo_) {
    throw std::runtime_error("cuber::Vbo::Create");
  }

  cuber::VertexLayout layouts[] = {
      {
          .location = vpos_location,
          .vbo = vbo,
          .type = GL_FLOAT,
          .count = 3,
          .offset = offsetof(Vertex, position),
          .stride = sizeof(Vertex),
      },
      {
          .location = vbarycentric_location,
          .vbo = vbo,
          .type = GL_FLOAT,
          .count = 3,
          .offset = offsetof(Vertex, barycentric),
          .stride = sizeof(Vertex),
      },
      //
      {
          .location = 2,
          .vbo = instance_vbo_,
          .type = GL_FLOAT,
          .count = 4,
          .offset = offsetof(Instance, row0),
          .stride = sizeof(Instance),
          .divisor = 1,
      },
      {
          .location = 3,
          .vbo = instance_vbo_,
          .type = GL_FLOAT,
          .count = 4,
          .offset = offsetof(Instance, row1),
          .stride = sizeof(Instance),
          .divisor = 1,
      },
      {
          .location = 4,
          .vbo = instance_vbo_,
          .type = GL_FLOAT,
          .count = 4,
          .offset = offsetof(Instance, row2),
          .stride = sizeof(Instance),
          .divisor = 1,
      },
      {
          .location = 5,
          .vbo = instance_vbo_,
          .type = GL_FLOAT,
          .count = 4,
          .offset = offsetof(Instance, row3),
          .stride = sizeof(Instance),
          .divisor = 1,
      },
  };
  auto ibo = cuber::Ibo::Create(sizeof(uint32_t) * builder.indices.size(),
                                builder.indices.data());
  if (!ibo) {
    throw std::runtime_error("cuber::Vbo::Create");
  }
  vao_ = cuber::Vao::Create(layouts, ibo);
  if (!vao_) {
    throw std::runtime_error("cuber::Vao::Create");
  }
}
CubeRenderer::~CubeRenderer() {}
void CubeRenderer::Render(const float projection[16], const float view[16],
                          const void *data, uint32_t instanceCount) {
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  auto v = DirectX::XMLoadFloat4x4((const DirectX::XMFLOAT4X4 *)view);
  auto p = DirectX::XMLoadFloat4x4((const DirectX::XMFLOAT4X4 *)projection);
  DirectX::XMFLOAT4X4 vp;
  DirectX::XMStoreFloat4x4(&vp, v * p);

  shader_->Bind();
  shader_->SetUniformMatrix([](auto err) {}, "VP", vp);

  instance_vbo_->Upload(sizeof(float) * 16 * instanceCount, data);
  vao_->DrawInstance(instanceCount, CUBE_INDEX_COUNT);
}

} // namespace cuber