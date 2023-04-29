#include <DirectXMath.h>
#include <GL/glew.h>
#include <cuber/gl3/GlLineRenderer.h>
#include <cuber/mesh.h>
#include <grapho/gl3/shader.h>
#include <grapho/gl3/vao.h>
#include <iostream>

using namespace grapho::gl3;

namespace cuber::gl3 {

static const char* vertex_shader_text = R"(
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

static const char* fragment_shader_text = R"(
in vec4 color;
out vec4 FragColor;

void main()
{
    FragColor = color;
}
)";

GlLineRenderer::GlLineRenderer()
{

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
  if (auto shader = ShaderProgram::Create(vs, fs)) {
    shader_ = *shader;
  } else {
    throw std::runtime_error(shader.error());
  }

  vbo_ = Vbo::Create(sizeof(LineVertex) * 65535, nullptr);
  if (!vbo_) {
    throw std::runtime_error("grapho::gl3::Vbo::Create");
  }

  VertexSlot slots[] = {
    { vbo_ }, //
  };
  grapho::VertexLayout layouts[] = {
      {
          .Id =
              {
                  .SemanticName = "vPos",
                  .Slot = 0,
                  .AttributeLocation = 0,
              },
          .Type = grapho::ValueType::Float,
          .Count = 3,
          .Offset = 0,
          .Stride = sizeof(LineVertex),
      },
      {
          .Id =
              {
                  .SemanticName = "vColor",
                  .Slot = 0,
                  .AttributeLocation = 1,
              },
          .Type = grapho::ValueType::Float,
          .Count = 4,
          .Offset = offsetof(LineVertex, Color),
          .Stride = sizeof(LineVertex),
      },
  };

  vao_ = Vao::Create(layouts, slots);
  if (!vao_) {
    throw std::runtime_error("grapho::gl3::Vao::Create");
  }
}
GlLineRenderer::~GlLineRenderer() {}
void
GlLineRenderer::Render(const float projection[16],
                       const float view[16],
                       std::span<const LineVertex> lines)
{
  if (lines.empty()) {
    return;
  }
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  auto v = DirectX::XMLoadFloat4x4((const DirectX::XMFLOAT4X4*)view);
  auto p = DirectX::XMLoadFloat4x4((const DirectX::XMFLOAT4X4*)projection);
  DirectX::XMFLOAT4X4 vp;
  DirectX::XMStoreFloat4x4(&vp, v * p);

  shader_->Bind();
  shader_->SetUniformMatrix("VP", vp);

  vbo_->Upload(sizeof(LineVertex) * lines.size(), lines.data());
  vao_->Draw(GL_LINES, lines.size(), 0);
}

} // namespace cuber::gl3
