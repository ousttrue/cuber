#pragma once
#include "Bvh.h"
#include <DirectXMath.h>
#include <cuber.h>
#include <list>
#include <memory>
#include <stack>
#include <vector>

struct BvhCurve {
  BvhChannelTypes chnnel;
  std::vector<float> values;
};

struct BvhNode {
  bool isRoot = false;
  BvhOffset localOffset;
  BvhCurve curves[6];
  std::list<std::shared_ptr<BvhNode>> children;
  BvhNode(const BvhOffset &offset) : localOffset(offset) {}
  static std::shared_ptr<BvhNode> Create(const BvhJoint &joint);
  int ChannelCount() const {
    int i = 0;
    for (; i < 6; ++i) {
      if (curves[i].chnnel == BvhChannelTypes::None) {
        break;
      }
    }
    return i;
  }
  void ResolveFrame(int frame, DirectX::XMMATRIX m,
                    std::span<cuber::Instance>::iterator &out);
};

class BvhSolver {
  std::vector<std::shared_ptr<BvhNode>> nodes_;
  std::shared_ptr<BvhNode> root_;
  std::vector<cuber::Instance> instances_;

public:
  void PushJoint(const BvhJoint &joint);
  void PushFrame(BvhTime time, std::span<const float> channelValues);
  std::span<cuber::Instance> GetFrame(int frame);

private:
  void PushFrame(std::span<const float>::iterator &it,
                 std::shared_ptr<BvhNode> node);
};
