#pragma once
#include "Bvh.h"
#include <DirectXMath.h>
#include <chrono>
#include <mutex>
#include <span>
#include <stdint.h>
#include <vector>
#include <winrt/base.h>

using RenderTime = std::chrono::duration<float, std::ratio<1, 1>>;

class DxRenderer {
  struct DxRendererImpl *impl_ = nullptr;
  std::vector<DirectX::XMFLOAT4X4> instancies_;
  std::mutex mutex_;

public:
  DxRenderer(const DxRenderer &) = delete;
  DxRenderer &operator=(const DxRenderer &) = delete;
  DxRenderer(const winrt::com_ptr<struct ID3D11Device> &device);
  ~DxRenderer();
  void SetBvh(const std::shared_ptr<Bvh> &bvh);
  void SyncFrame(const BvhFrame &frame);
  void RenderScene(RenderTime time, const float projection[16],
                   const float view[16]);
};
