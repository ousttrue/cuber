#pragma once
#include <array>
#include <memory>
#include <span>

namespace cuber {

template <typename T>
concept Float16 = sizeof(T) == sizeof(float) * 16;

class DxCubeRenderer {
  struct DxCubeRendererImpl *impl_ = nullptr;

public:
  DxCubeRenderer(const DxCubeRenderer &) = delete;
  DxCubeRenderer &operator=(const DxCubeRenderer &) = delete;
  DxCubeRenderer();
  ~DxCubeRenderer();
  void Render(const float projection[16], const float view[16],
              const void *data, uint32_t instanceCount);
  template <Float16 T>
  void Render(const float projection[16], const float view[16],
              std::span<const T> instances) {
    Render(projection, view, instances.data(), instances.size());
  }
};
} // namespace cuber