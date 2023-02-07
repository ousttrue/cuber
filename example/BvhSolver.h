#pragma once
#include "Bvh.h"
#include <DirectXMath.h>
#include <list>
#include <memory>
#include <stack>
#include <vector>

class BvhNode;
class BvhSolver {
  std::vector<std::shared_ptr<BvhNode>> nodes_;
  std::shared_ptr<BvhNode> root_;
  std::vector<DirectX::XMFLOAT4X4> instances_;
  float scaling_ = 1.0f;

public:
  void Initialize(const std::shared_ptr<Bvh> &bvh);
  std::span<DirectX::XMFLOAT4X4> GetFrame(int frame);

private:
  void PushJoint(const BvhJoint &joint);
  void CalcShape();
  void PushFrame(const BvhFrame &time);

  void PushFrame(std::span<const float>::iterator &it,
                 std::shared_ptr<BvhNode> node);
};
