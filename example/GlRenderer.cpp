#include "GlRenderer.h"
#include "Bvh.h"
#include <DirectXMath.h>
#include <GL/glew.h>
#include <cuber.h>
#include <fstream>
#include <iostream>

template <typename T>
static std::vector<T> ReadAllBytes(const std::string &filename) {
  std::ifstream ifs(filename.c_str(), std::ios::binary | std::ios::ate);
  if (!ifs) {
    return {};
  }
  auto pos = ifs.tellg();
  auto size = pos / sizeof(T);
  if (pos % sizeof(T)) {
    ++size;
  }
  std::vector<T> buffer(size);
  ifs.seekg(0, std::ios::beg);
  ifs.read(buffer.data(), pos);
  return buffer;
}

static const struct {
  float x, y;
  float r, g, b;
} vertices[3] = {{-0.6f, -0.4f, 1.f, 0.f, 0.f},
                 {0.6f, -0.4f, 0.f, 1.f, 0.f},
                 {0.f, 0.6f, 0.f, 0.f, 1.f}};

static const char *vertex_shader_text = R"(
uniform mat4 MVP;
in vec2 vPos;
in vec3 vCol;
out vec3 color;
void main()
{
    gl_Position = MVP * vec4(vPos, 0.0, 1.0);
    color = vCol;
}
)";

static const char *fragment_shader_text = R"(
in vec3 color;
out vec4 FragColor;
void main()
{
    FragColor = vec4(color, 1.0);
}
)";

struct GlRendererImpl {
  std::shared_ptr<cuber::Vao> vao_;
  std::shared_ptr<cuber::ShaderProgram> shader_;
  cuber::CubeRenderer cubes;

  GlRendererImpl() {

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

    auto vbo = cuber::Vbo::Create(sizeof(vertices), vertices);
    if (!vbo) {
      throw std::runtime_error("cuber::Vbo::Create");
    }
    cuber::VertexLayout layouts[] = {
        {
            .type = GL_FLOAT,
            .count = 2,
            .offset = 0,
            .stride = sizeof(vertices[0]),
        },
        {
            .type = GL_FLOAT,
            .count = 3,
            .offset = 8,
            .stride = sizeof(vertices[0]),
        },
    };
    vao_ = cuber::Vao::Create(vbo, layouts);
    if (!vao_) {
      throw std::runtime_error("cuber::Vao::Create");
    }

    cubes.Push(0.5f, {});
  }
  ~GlRendererImpl() {}

  void Render(RenderTime time, const float projection[16],
              const float view[16]) {
    auto angle = DirectX::XMConvertToRadians(time.count()) * 10.0f;
    auto m = DirectX::XMMatrixRotationZ(angle);
    auto v = DirectX::XMLoadFloat4x4((const DirectX::XMFLOAT4X4 *)view);
    auto p = DirectX::XMLoadFloat4x4((const DirectX::XMFLOAT4X4 *)projection);
    DirectX::XMFLOAT4X4 mvp;
    DirectX::XMStoreFloat4x4(&mvp, m * v * p);

    shader_->Bind();
    shader_->SetUniformMatrix([](auto err) {}, "MVP", mvp);

    vao_->Draw(0, 3);

    cubes.Render(projection, view);
  }
};

GlRenderer::GlRenderer() {
  std::cout << "GL_VERSION: " << glGetString(GL_VERSION) << std::endl;
  glewInit();
  std::cout << "GLEW_VERSION: " << glewGetString(GLEW_VERSION) << std::endl;
  impl_ = new GlRendererImpl;
}

GlRenderer::~GlRenderer() { delete impl_; }

void GlRenderer::Load(std::string_view file) {

  auto bytes = ReadAllBytes<char>(std::string(file.begin(), file.end()));
  if (bytes.empty()) {
    return;
  }
  std::cout << "load: " << file << " " << bytes.size() << "bytes" << std::endl;
  Bvh bvh;
  if (!bvh.Parse({bytes.begin(), bytes.end()})) {
    return;
  }
  std::cout << bvh << std::endl;
}

void GlRenderer::RenderScene(RenderTime time, const float projection[16],
                             const float view[16]) {
  impl_->Render(time, projection, view);
}
