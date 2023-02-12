#pragma once
#include <cuber/concept.h>
#include <array>
#include <memory>
#include <span>
#include <winrt/base.h>

struct ID3D11Device;

namespace cuber {

class DxCubeRenderer {
  struct DxCubeRendererImpl *impl_ = nullptr;

public:
  DxCubeRenderer(const DxCubeRenderer &) = delete;
  DxCubeRenderer &operator=(const DxCubeRenderer &) = delete;
  DxCubeRenderer(const winrt::com_ptr<ID3D11Device> &device);
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