#include "BvhSolver.h"
#include "BvhNode.h"
#include <assert.h>
#include <iostream>

//
// BvhSolver
//
void BvhSolver::PushJoint(const BvhJoint &joint, float scaling) {

  auto node = BvhNode::Create(joint, scaling, nodes_.empty());
  nodes_.push_back(node);
  instances_.push_back({});

  if (nodes_.size() == 1) {
    root_ = node;
  } else {
    auto parent = nodes_[joint.parent];
    parent->AddChild(node);
  }
}

void BvhSolver::PushFrame(BvhTime time, std::span<const float> channelValues,
                          float scaling) {
  auto it = channelValues.begin();
  root_->PushFrame(it, scaling);
  assert(it == channelValues.end());
}

std::span<cuber::Instance> BvhSolver::GetFrame(int frame) {
  auto span = std::span(instances_);
  auto it = span.begin();
  root_->ResolveFrame(frame, DirectX::XMMatrixIdentity(), it);
  assert(it == span.end());
  return instances_;
}
