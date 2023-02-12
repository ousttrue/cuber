#include <DirectXMath.h>
#include <GL/glew.h>
#include <cuber/gl3/GlLineRenderer.h>
#include <cuber/gl3/shader.h>
#include <cuber/gl3/vao.h>
#include <cuber/mesh.h>
#include <iostream>

namespace cuber::gl3 {

static const char *vertex_shader_text = R"(
uniform mat4 VP;
in vec3 vPos;
in vec4 vColor;
out vec4 color;

void main()
{
    gl_Position = VP * vec4(vPos, 1.0);
    color = vColor;
}
)";

static const char *fragment_shader_text = R"(
in vec4 color;
out vec4 FragColor;

void main()
{
    FragColor = color;
}
)";

static uint32_t get_location(const std::shared_ptr<ShaderProgram> &shader,
                             const char *name) {
  auto location = shader->AttributeLocation(name);
  if (!location) {
    throw std::runtime_error("glGetUniformLocation");
  }
  return *location;
}

GlLineRenderer::GlLineRenderer() {

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
  shader_ = ShaderProgram::Create(
      [](auto msg) { std::cout << msg << std::endl; }, vs, fs);
  if (!shader_) {
    throw std::runtime_error("cuber::ShaderProgram::Create");
  }

  // auto vpos_location = get_location(shader_, "vPos");
  // auto vbarycentric_location = get_location(shader_, "vBarycentric");
  // auto ipos_location = get_location(shader_, "iPos");
  // auto irot_location = get_location(shader_, "iRot");

  vbo_ = Vbo::Create(sizeof(LineVertex) * 65535, nullptr);
  if (!vbo_) {
    throw std::runtime_error("cuber::Vbo::Create");
  }

  VertexLayout layouts[] = {
      {
          .id =
              {
                  .semantic_name = "vPos",
              },
          .type = cuber::ValueType::Float,
          .count = 3,
          .offset = 0,
          .stride = sizeof(LineVertex),
      },
      {
          .id =
              {
                  .semantic_name = "vColor",
              },
          .type = cuber::ValueType::Float,
          .count = 4,
          .offset = offsetof(LineVertex, color),
          .stride = sizeof(LineVertex),
      },
  };
  VertexSlot slots[] = {
      {0, vbo_}, //
      {1, vbo_}, //
  };

  vao_ = Vao::Create(layouts, slots);
  if (!vao_) {
    throw std::runtime_error("cuber::Vao::Create");
  }
}
GlLineRenderer::~GlLineRenderer() {}
void GlLineRenderer::Render(const float projection[16], const float view[16],
                            std::span<const LineVertex> lines) {
  if (lines.empty()) {
    return;
  }
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  auto v = DirectX::XMLoadFloat4x4((const DirectX::XMFLOAT4X4 *)view);
  auto p = DirectX::XMLoadFloat4x4((const DirectX::XMFLOAT4X4 *)projection);
  DirectX::XMFLOAT4X4 vp;
  DirectX::XMStoreFloat4x4(&vp, v * p);

  shader_->Bind();
  shader_->SetUniformMatrix([](auto err) {}, "VP", vp);

  vbo_->Upload(sizeof(LineVertex) * lines.size(), lines.data());
  vao_->Draw(GL_LINES, lines.size(), 0);
}

} // namespace cuber::gl3