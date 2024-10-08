#include <DirectXMath.h>

#include "BvhNode.h"
#include "BvhSolver.h"
#include <assert.h>
#include <iostream>

void
BvhSolver::Initialize(const std::shared_ptr<Bvh>& bvh)
{
  nodes_.clear();
  root_.reset();
  instances_.clear();

  scaling_ = bvh->GuessScaling();
  for (auto& joint : bvh->joints) {
    PushJoint(joint);
  };
  CalcShape();
}

void
BvhSolver::PushJoint(BvhJoint& joint)
{
  auto node = std::make_shared<BvhNode>(joint);
  nodes_.push_back(node);
  instances_.push_back({});

  if (nodes_.size() == 1) {
    root_ = node;
  } else {
    auto parent = nodes_[joint.parent];
    parent->AddChild(node);
  }
}

void
BvhSolver::CalcShape()
{
  root_->CalcShape(scaling_);
}

std::span<grapho::XMFLOAT4X4>
BvhSolver::ResolveFrame(const BvhFrame& frame)
{
  auto span = std::span(instances_);
  auto it = span.begin();
  root_->ResolveFrame(frame,
                      {
                        1,
                        0,
                        0,
                        0,
                        /**/ 0,
                        1,
                        0,
                        0,
                        /**/ 0,
                        0,
                        1,
                        0,
                        /**/ 0,
                        0,
                        0,
                        1,
                      },
                      scaling_,
                      it);
  assert(it == span.end());
  return instances_;
}
