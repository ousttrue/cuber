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
  std::string name_;
  bool isRoot_ = false;
  DirectX::SimpleMath::Vector3 localOffset_;
  BvhCurve curves_[6];
  std::list<std::shared_ptr<BvhNode>> children_;
  DirectX::XMFLOAT4X4 shape_;

public:
  BvhNode(std::string_view name, const DirectX::XMFLOAT3 &offset, bool isRoot);
  static std::shared_ptr<BvhNode> Create(const BvhJoint &joint, float scaling,
                                         bool isRoot);
  int ChannelCount() const {
    int i = 0;
    for (; i < 6; ++i) {
      if (curves_[i].chnnel == BvhChannelTypes::None) {
        break;
      }
    }
    return i;
  }
  void AddChild(const std::shared_ptr<BvhNode> &node) {
    children_.push_back(node);
  }
  void CalcShape();
  void PushFrame(std::span<const float>::iterator &it, float scaling);
  void ResolveFrame(int frame, DirectX::XMMATRIX m,
                    std::span<DirectX::XMFLOAT4X4>::iterator &out);
};
