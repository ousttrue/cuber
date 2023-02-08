#pragma once
#include "Bvh.h"
#include <DirectXMath.h>
#include <chrono>
#include <cuber.h>
#include <mutex>
#include <span>
#include <stdint.h>
#include <vector>

using RenderTime = std::chrono::duration<float, std::ratio<1, 1>>;

class GlRenderer {
  struct GlRendererImpl *impl_ = nullptr;
  cuber::CubeRenderer cubes;
  std::vector<DirectX::XMFLOAT4X4> instancies_;
  std::mutex mutex_;

public:
  GlRenderer(const GlRenderer &) = delete;
  GlRenderer &operator=(const GlRenderer &) = delete;
  GlRenderer();
  ~GlRenderer();
  void SetBvh(const std::shared_ptr<Bvh> &bvh);
  void SetFrame(const BvhFrame &frame);
  void RenderScene(RenderTime time, const float projection[16],
                   const float view[16]);
};
