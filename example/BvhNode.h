#pragma once
#include "Bvh.h"
#include <SimpleMath.h>
#include <cuber.h>
#include <list>
#include <memory>
#include <vector>

struct BvhCurve {
  BvhChannelTypes chnnel;
  std::vector<float> values;
};

class BvhNode {
  bool isRoot_ = false;
  BvhOffset localOffset_;
  BvhCurve curves_[6];
  std::list<std::shared_ptr<BvhNode>> children_;
  DirectX::XMFLOAT4X4 shape_;

public:
  BvhNode(const BvhOffset &offset, bool isRoot)
      : isRoot_(isRoot), localOffset_(offset) {}
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
  void PushFrame(std::span<const float>::iterator &it, float scaling) {

    for (int i = 0; i < ChannelCount(); ++i, ++it) {
      curves_[i].values.push_back(*it);
    }
    for (auto &child : children_) {
      child->PushFrame(it, scaling);
    }
  }
  void ResolveFrame(int frame, DirectX::XMMATRIX m,
                    std::span<cuber::Instance>::iterator &out);
};
