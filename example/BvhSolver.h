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

public:
  void Initialize() {
    nodes_.clear();
    root_.reset();
    instances_.clear();
  }
  void PushJoint(const BvhJoint &joint, float scaling);
  void CalcShape();
  void PushFrame(BvhTime time, std::span<const float> channelValues,
                 float scaling);
  std::span<DirectX::XMFLOAT4X4> GetFrame(int frame);

private:
  void PushFrame(std::span<const float>::iterator &it,
                 std::shared_ptr<BvhNode> node);
};
