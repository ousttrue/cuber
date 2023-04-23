#pragma once
#include <array>
#include <cuber/mesh.h>
#include <memory>
#include <span>
#include <winrt/base.h>

struct ID3D11Device;

namespace cuber::dx11 {

template<typename T>
concept Float16 = sizeof(T) == sizeof(float) * 16;

class DxCubeStereoRenderer
{
  struct DxCubeStereoRendererImpl* impl_ = nullptr;

public:
  DxCubeStereoRenderer(const DxCubeStereoRenderer&) = delete;
  DxCubeStereoRenderer& operator=(const DxCubeStereoRenderer&) = delete;
  DxCubeStereoRenderer(const winrt::com_ptr<ID3D11Device>& device);
  ~DxCubeStereoRenderer();
  void Render(const float projection[16],
              const float view[16],
              const float rightProjection[16],
              const float rightView[16],
              const Instance* data,
              uint32_t instanceCount);
};

} // namespace cuber
