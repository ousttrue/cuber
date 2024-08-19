#pragma once
#include "Bvh.h"
#include <chrono>
#include <cuber/mesh.h>
#include <span>

using RenderTime = std::chrono::duration<float, std::ratio<1, 1>>;

class BvhPanel
{
  struct BvhPanelImpl* m_impl = nullptr;

public:
  BvhPanel();
  ~BvhPanel();
  void SetBvh(const std::shared_ptr<Bvh>& bvh);
  void UpdateGui();
  std::span<const cuber::Instance> GetCubes();
  void GetCubes(std::vector<cuber::Instance>& cubes);
};
