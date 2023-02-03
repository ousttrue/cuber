#include "CubeRenderer.h"
#include <DirectXMath.h>
#include <GL/glew.h>
#include <iostream>

static const char *vertex_shader_text = R"(
uniform mat4 VP;
in vec3 vPos;
in vec4 vCol;
in vec3 iPos;
// in vec4 iRot;
out vec4 color;

mat4 translation(vec3 t)
{
  return mat4(
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    t.x, t.y, t.z, 1
  );
}

void main()
{
    gl_Position = VP * translation(iPos) * vec4(vPos, 1.0);
    color = vCol;
}
)";

static const char *fragment_shader_text = R"(
in vec4 color;
out vec4 FragColor;
void main()
{
    FragColor = vec4(color);
}
)";

namespace cuber {

const float s = 0.5f;
xyz positions[8] = {
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
  rgba color;
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

struct Vertex {
  xyz position;
  rgba color;
};

struct Builder {
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
  void Quad(const xyz &p0, const xyz &p1, const xyz &p2, const xyz &p3,
            const rgba &color) {
    Vertex v0{
        .position = p0,
        .color = color,
    };
    Vertex v1{
        .position = p1,
        .color = color,
    };
    Vertex v2{
        .position = p2,
        .color = color,
    };
    Vertex v3{
        .position = p3,
        .color = color,
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
  auto vcol_location = get_location(shader_, "vCol");
  auto ipos_location = get_location(shader_, "iPos");
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

  instance_vbo_ = cuber::Vbo::Create(sizeof(Instance) * 65535);
  if (!instance_vbo_) {
    throw std::runtime_error("cuber::Vbo::Create");
  }

  cuber::VertexLayout layouts[] = {
      {
          .location = vpos_location,
          .vbo = vbo,
          .type = GL_FLOAT,
          .count = 3,
          .offset = 0,
          .stride = sizeof(Vertex),
      },
      {
          .location = vcol_location,
          .vbo = vbo,
          .type = GL_FLOAT,
          .count = 4,
          .offset = 12,
          .stride = sizeof(Vertex),
      },
      {
          .location = ipos_location,
          .vbo = instance_vbo_,
          .type = GL_FLOAT,
          .count = 3,
          .offset = 0,
          .stride = sizeof(Instance),
          .divisor = 1,
      },
      // {
      //     .location = irot_location,
      //     .vbo = instance_vbo_,
      //     .type = GL_FLOAT,
      //     .count = 4,
      //     .offset = 12,
      //     .stride = sizeof(Instance),
      //     .divisor = 1,
      // },
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

  cubes_.push_back({
      .position = {1, 0, 0},
  });
  cubes_.push_back({
      .position = {-1, 1, 0},
  });

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
}
CubeRenderer::~CubeRenderer() {}
void CubeRenderer::Push(float size, const xyz &pos) {}
void CubeRenderer::Render(const float projection[16], const float view[16]) {
  auto v = DirectX::XMLoadFloat4x4((const DirectX::XMFLOAT4X4 *)view);
  auto p = DirectX::XMLoadFloat4x4((const DirectX::XMFLOAT4X4 *)projection);
  DirectX::XMFLOAT4X4 vp;
  DirectX::XMStoreFloat4x4(&vp, v * p);

  shader_->Bind();
  shader_->SetUniformMatrix([](auto err) {}, "VP", vp);

  instance_vbo_->Upload(sizeof(Instance) * cubes_.size(), cubes_.data());
  vao_->DrawInstance(cubes_.size(), cubes_.size() * 36);
}

} // namespace cuber