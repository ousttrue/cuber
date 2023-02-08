#pragma once
#include "shader.h"
#include "vao.h"
#include <array>
#include <memory>

namespace cuber {

template <typename T>
concept Float16 = sizeof(T) == sizeof(float) * 16;

class CubeRenderer {
  std::shared_ptr<Vao> vao_;
  std::shared_ptr<ShaderProgram> shader_;
  std::shared_ptr<Vbo> instance_vbo_;

public:
  CubeRenderer(const CubeRenderer &) = delete;
  CubeRenderer &operator=(const CubeRenderer &) = delete;
  CubeRenderer();
  ~CubeRenderer();
  void Render(const float projection[16], const float view[16],
              const void *data, uint32_t instanceCount);
  template <Float16 T>
  void Render(const float projection[16], const float view[16],
              std::span<const T> instances) {
    Render(projection, view, instances.data(), instances.size());
  }
};
} // namespace cuber