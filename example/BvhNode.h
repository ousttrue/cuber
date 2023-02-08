#pragma once
#include "Bvh.h"
#include <SimpleMath.h>
#include <list>
#include <memory>
#include <vector>

struct BvhCurve {
  BvhChannelTypes chnnel;
  std::vector<float> values;
};

class BvhNode {
  const BvhJoint &joint_;
  BvhCurve curves_[6];
  std::list<std::shared_ptr<BvhNode>> children_;
  DirectX::XMFLOAT4X4 shape_;

public:
  BvhNode(const BvhJoint &joint);
  void AddChild(const std::shared_ptr<BvhNode> &node) {
    children_.push_back(node);
  }
  void CalcShape(float scaling);
  void PushFrame(std::span<const float>::iterator &it);
  void ResolveFrame(const BvhFrame &frame, DirectX::XMMATRIX m, float scaling,
                    std::span<DirectX::XMFLOAT4X4>::iterator &out);
};
