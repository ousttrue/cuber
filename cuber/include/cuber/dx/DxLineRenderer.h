#pragma once
#include <DirectXMath.h>
#include <array>
#include <memory>
#include <span>
#include <winrt/base.h>

struct ID3D11Device;

namespace cuber {

struct LineVertex {
  DirectX::XMFLOAT3 position;
  DirectX::XMFLOAT4 color;
};
static_assert(sizeof(LineVertex) == 28);

void PushGrid(std::vector<LineVertex> &lines, float interval = 1.0f,
              int half_count = 5);

class DxLineRenderer {
  struct DxLineRendererImpl *impl_ = nullptr;

public:
  DxLineRenderer(const DxLineRenderer &) = delete;
  DxLineRenderer &operator=(const DxLineRenderer &) = delete;
  DxLineRenderer(const winrt::com_ptr<ID3D11Device> &device);
  ~DxLineRenderer();
  void Render(const float projection[16], const float view[16],
              std::span<const LineVertex> data);
};

} // namespace cuber