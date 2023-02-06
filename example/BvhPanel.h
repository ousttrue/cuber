#pragma once
#include <DirectXMath.h>
#include <chrono>
#include <functional>
#include <span>
#include <string_view>

class BvhPanel {
  class BvhPanelImpl *impl_ = nullptr;

public:
  BvhPanel();
  ~BvhPanel();
  bool LoadBvh(std::string_view path);
  void
  OnFrame(const std::function<void(std::chrono::nanoseconds,
                                   std::span<DirectX::XMFLOAT4X4>)> &onFrame);
  void UpdateGui();
};
