#pragma once
#include <DirectXMath.h>
#include <array>
#include <cuber/mesh.h>
#include <memory>
#include <span>
#include <winrt/base.h>

namespace grapho::gl3 {
class Vbo;
struct Vao;
class ShaderProgram;
} // namespace grapho::gl3

namespace cuber::gl3 {

class GlLineRenderer {
  std::shared_ptr<grapho::gl3::Vbo> vbo_;
  std::shared_ptr<grapho::gl3::Vao> vao_;
  std::shared_ptr<grapho::gl3::ShaderProgram> shader_;

public:
  GlLineRenderer(const GlLineRenderer &) = delete;
  GlLineRenderer &operator=(const GlLineRenderer &) = delete;
  GlLineRenderer();
  ~GlLineRenderer();
  void Render(const float projection[16], const float view[16],
              std::span<const grapho::LineVertex> data);
};

} // namespace cuber::gl3
