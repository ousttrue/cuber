#include "BvhSolver.h"
#include "BvhNode.h"
#include <assert.h>
#include <iostream>

void BvhSolver::Initialize(const std::shared_ptr<Bvh> &bvh) {
  nodes_.clear();
  root_.reset();
  instances_.clear();

  scaling_ = bvh->GuessScaling();
  for (auto &joint : bvh->joints) {
    PushJoint(joint);
  };
  CalcShape();

  int frameCount = bvh->FrameCount();
  for (int i = 0; i < frameCount; ++i) {
    auto frame = bvh->GetFrame(i);
    // auto time = bvh_->frame_time * i;
    PushFrame(frame);
  }
}

void BvhSolver::PushJoint(const BvhJoint &joint) {
  auto node = BvhNode::Create(joint, scaling_);
  nodes_.push_back(node);
  instances_.push_back({});

  if (nodes_.size() == 1) {
    root_ = node;
  } else {
    auto parent = nodes_[joint.parent];
    parent->AddChild(node);
  }
}

void BvhSolver::CalcShape() { root_->CalcShape(); }

void BvhSolver::PushFrame(const BvhFrame &frame) {
  auto it = frame.values.begin();
  root_->PushFrame(it, scaling_);
  assert(it == frame.values.end());
}

std::span<DirectX::XMFLOAT4X4> BvhSolver::ResolveFrame(const BvhFrame &frame) {
  auto span = std::span(instances_);
  auto it = span.begin();
  root_->ResolveFrame(frame, DirectX::XMMatrixIdentity(), it);
  assert(it == span.end());
  return instances_;
}
