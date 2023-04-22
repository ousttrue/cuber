#pragma once
#include <array>
#include <memory>
#include <span>
#include "cuber/concept.h"

namespace grapho::gl3 {
struct Vao;
class ShaderProgram;
class Vbo;
} // namespace grapho::gl3

namespace cuber::gl3 {

class GlCubeRenderer {
  std::shared_ptr<grapho::gl3::Vao> m_vao;
  std::shared_ptr<grapho::gl3::ShaderProgram> m_shader;
  std::shared_ptr<grapho::gl3::Vbo> m_instance_vbo;
  std::shared_ptr<grapho::gl3::Vbo> m_attribute_vbo;

public:
  GlCubeRenderer(const GlCubeRenderer &) = delete;
  GlCubeRenderer &operator=(const GlCubeRenderer &) = delete;
  GlCubeRenderer();
  ~GlCubeRenderer();
  void Render(const float projection[16], const float view[16],
              const void *data, uint32_t instanceCount,
              const void *attributes = nullptr);

  template <Float16 T>
  void Render(const float projection[16], const float view[16],
              std::span<const T> instances) {
    Render(projection, view, instances.data(), instances.size());
  }

  template <typename T, typename A>
    requires Float16_4<T, A>
  void Render(const float projection[16], const float view[16],
              std::span<const T> instances, std::span<const A> attributes) {
    Render(projection, view, instances.data(), instances.size(),
           attributes.data());
  }
};
} // namespace cuber::gl3
