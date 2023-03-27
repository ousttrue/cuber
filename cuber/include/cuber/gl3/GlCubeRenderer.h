#pragma once
#include <array>
#include <memory>
#include <span>

namespace grapho::gl3 {
struct Vao;
class ShaderProgram;
class Vbo;
} // namespace grapho::gl3

namespace cuber::gl3 {

template <typename T>
concept Float16 = sizeof(T) == sizeof(float) * 16;
class GlCubeRenderer {
  std::shared_ptr<grapho::gl3::Vao> vao_;
  std::shared_ptr<grapho::gl3::ShaderProgram> shader_;
  std::shared_ptr<grapho::gl3::Vbo> instance_vbo_;

public:
  GlCubeRenderer(const GlCubeRenderer &) = delete;
  GlCubeRenderer &operator=(const GlCubeRenderer &) = delete;
  GlCubeRenderer();
  ~GlCubeRenderer();
  void Render(const float projection[16], const float view[16],
              const void *data, uint32_t instanceCount);
  template <Float16 T>
  void Render(const float projection[16], const float view[16],
              std::span<const T> instances) {
    Render(projection, view, instances.data(), instances.size());
  }
};
} // namespace cuber::gl3
