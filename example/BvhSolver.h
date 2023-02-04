#pragma once
#include "Bvh.h"
#include <DirectXMath.h>
#include <cuber.h>
#include <list>
#include <memory>
#include <stack>
#include <vector>

class BvhNode;
class BvhSolver {
  std::vector<std::shared_ptr<BvhNode>> nodes_;
  std::shared_ptr<BvhNode> root_;
  std::vector<cuber::Instance> instances_;

public:
  void Initialize() {
    nodes_.clear();
    root_.reset();
    instances_.clear();
  }
  void PushJoint(const BvhJoint &joint, float scaling);
  void PushFrame(BvhTime time, std::span<const float> channelValues,
                 float scaling);
  std::span<cuber::Instance> GetFrame(int frame);

private:
  void PushFrame(std::span<const float>::iterator &it,
                 std::shared_ptr<BvhNode> node);
};
