#pragma once
#include "Bvh.h"
#include "srht.h"
#include <list>
#include <memory>
#include <vector>

struct BvhNode {
  const BvhJoint &joint_;
  DirectX::XMFLOAT4X4 shape_;
  srht::HumanoidBones bone_ = {};
  std::list<std::shared_ptr<BvhNode>> children_;
  BvhNode(const BvhJoint &joint);
  void AddChild(const std::shared_ptr<BvhNode> &node) {
    children_.push_back(node);
  }
  void CalcShape(float scaling);
  void ResolveFrame(const BvhFrame &frame, DirectX::XMMATRIX m, float scaling,
                    std::span<DirectX::XMFLOAT4X4>::iterator &out);
};
