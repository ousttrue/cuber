#include <DirectXMath.h>
#include <GL/glew.h>
#include <cuber/gl3/GlCubeRenderer.h>
#include <cuber/mesh.h>
#include <grapho/gl3/shader.h>
#include <grapho/gl3/vao.h>
#include <iostream>

using namespace grapho::gl3;

namespace cuber::gl3 {

static const char *vertex_m_shadertext = R"(
uniform mat4 VP;
in vec3 vPos;
in vec2 vBarycentric;
in vec4 iRow0;
in vec4 iRow1;
in vec4 iRow2;
in vec4 iRow3;
in vec4 iColor;
out vec2 oBarycentric;
out vec4 oColor;

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
    oBarycentric = vBarycentric;
    oColor = iColor;
}
)";

static const char *fragment_m_shadertext = R"(
in vec2 oBarycentric;
in vec4 oColor;
out vec4 FragColor;

// https://github.com/rreusser/glsl-solid-wireframe
float grid (vec2 vBC, float width) {
  vec3 bary = vec3(vBC.x, vBC.y, 1.0 - vBC.x - vBC.y);
  vec3 d = fwidth(bary);
  vec3 a3 = smoothstep(d * (width - 0.5), d * (width + 0.5), bary);
  return min(a3.x, a3.y);
}

void main()
{
    FragColor = oColor * vec4(vec3(grid(oBarycentric, 1.0)), 1);
}
)";

// static uint32_t get_location(const std::shared_ptr<ShaderProgram> &shader,
//                              const char *name) {
//   auto location = shader->AttributeLocation(name);
//   if (!location) {
//     throw std::runtime_error("glGetUniformLocation");
//   }
//   return *location;
// }

GlCubeRenderer::GlCubeRenderer() {

  // auto glsl_version = "#version 150";
  auto glsl_version = "#version 310 es\nprecision highp float;";

  std::string_view vs[] = {
      glsl_version,
      "\n",
      vertex_m_shadertext,
  };
  std::string_view fs[] = {
      glsl_version,
      "\n",
      fragment_m_shadertext,
  };
  if (auto shader = ShaderProgram::Create(vs, fs)) {
    m_shader = *shader;
  } else {
    throw std::runtime_error(shader.error());
  }

  auto [vertices, indices, layouts] = Cube(true, false);

  auto vbo =
      Vbo::Create(sizeof(grapho::Vertex) * vertices.size(), vertices.data());
  if (!vbo) {
    throw std::runtime_error("cuber::Vbo::Create");
  }

  m_instance_vbo = Vbo::Create(sizeof(float) * 16 * 65535, nullptr);
  if (!m_instance_vbo) {
    throw std::runtime_error("cuber::Vbo::Create: m_instance_vbo");
  }

  m_attribute_vbo = Vbo::Create(sizeof(float) * 4 * 65535, nullptr);
  if (!m_attribute_vbo) {
    throw std::runtime_error("cuber::Vbo::Create: m_attribute_vbo");
  }

  VertexSlot slots[] = {
      {0, vbo},             //
      {1, vbo},             //
      {2, m_instance_vbo},  //
      {3, m_instance_vbo},  //
      {4, m_instance_vbo},  //
      {5, m_instance_vbo},  //
      {6, m_attribute_vbo}, //
  };

  auto ibo = Ibo::Create(sizeof(uint32_t) * indices.size(), indices.data(),
                         GL_UNSIGNED_INT);
  if (!ibo) {
    throw std::runtime_error("cuber::Vbo::Create");
  }
  m_vao = Vao::Create(layouts, slots, ibo);
  if (!m_vao) {
    throw std::runtime_error("cuber::Vao::Create");
  }
}

GlCubeRenderer::~GlCubeRenderer() {}

void GlCubeRenderer::Render(const float projection[16], const float view[16],
                            const void *data, uint32_t instanceCount,
                            const void *attributes) {
  if (instanceCount == 0) {
    return;
  }
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  auto v = DirectX::XMLoadFloat4x4((const DirectX::XMFLOAT4X4 *)view);
  auto p = DirectX::XMLoadFloat4x4((const DirectX::XMFLOAT4X4 *)projection);
  DirectX::XMFLOAT4X4 vp;
  DirectX::XMStoreFloat4x4(&vp, v * p);

  m_shader->Bind();
  m_shader->SetUniformMatrix("VP", vp);

  m_instance_vbo->Upload(sizeof(float) * 16 * instanceCount, data);
  if (attributes) {
    m_attribute_vbo->Upload(sizeof(float) * 4 * instanceCount, attributes);
  }
  m_vao->DrawInstance(instanceCount, CUBE_INDEX_COUNT, 0);
}

} // namespace cuber::gl3
