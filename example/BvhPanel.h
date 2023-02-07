#pragma once
#include "Bvh.h"
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
  void SetBvh(const std::shared_ptr<Bvh> &bvh);
  void OnFrame(const std::function<void(const BvhFrame &frame)> &onFrame);
  void UpdateGui();
};
