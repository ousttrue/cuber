#pragma once
#include <DirectXMath.h>
#include <array>
#include <cuber/mesh.h>
#include <memory>
#include <span>
#include <winrt/base.h>

namespace cuber::gl3 {

class GlLineRenderer {
  std::shared_ptr<class Vbo> vbo_;
  std::shared_ptr<class Vao> vao_;
  std::shared_ptr<class ShaderProgram> shader_;

public:
  GlLineRenderer(const GlLineRenderer &) = delete;
  GlLineRenderer &operator=(const GlLineRenderer &) = delete;
  GlLineRenderer();
  ~GlLineRenderer();
  void Render(const float projection[16], const float view[16],
              std::span<const LineVertex> data);
};

} // namespace cuber::dx11