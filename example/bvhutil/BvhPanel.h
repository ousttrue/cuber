#pragma once
#include "Bvh.h"
#include <DirectXMath.h>
#include <chrono>
#include <functional>
#include <span>
#include <string_view>

using RenderTime = std::chrono::duration<float, std::ratio<1, 1>>;

class BvhPanel {
  struct BvhPanelImpl *m_impl = nullptr;
  std::vector<DirectX::XMFLOAT4> m_attributes;

public:
  BvhPanel();
  ~BvhPanel();
  void SetBvh(const std::shared_ptr<Bvh> &bvh);
  void UpdateGui();
  std::span<const DirectX::XMFLOAT4X4> GetCubes();

  // colors
  std::span<const DirectX::XMFLOAT4> GetCubeAttributes() {
    return m_attributes;
  }
};
