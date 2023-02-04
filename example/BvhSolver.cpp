#include "BvhSolver.h"
#include <assert.h>
#include <iostream>

//
// BvhNode
//
std::shared_ptr<BvhNode> BvhNode::Create(const BvhJoint &joint) {
  auto ptr = std::shared_ptr<BvhNode>(new BvhNode(joint.localOffset));
  for (int i = 0; i < 6; ++i) {
    ptr->curves[i].chnnel = joint.channels.values[i];
  }
  return ptr;
}

// [x, y, z][c6][c5][c4][c3][c2][c1][parent][root]
void BvhNode::ResolveFrame(int frame, DirectX::XMMATRIX m, float scaling,
                           std::span<cuber::Instance>::iterator &out) {

  auto local = isRoot ? DirectX::XMMatrixIdentity()
                      : DirectX::XMMatrixTranslation(localOffset.x * scaling,
                                                     localOffset.y * scaling,
                                                     localOffset.z * scaling);
  for (int i = 0; i < 6; ++i) {
    auto &curve = curves[i];
    switch (curve.chnnel) {
    case BvhChannelTypes::Xposition:
      if (isRoot) {
        local =
            DirectX::XMMatrixTranslation(curve.values[frame] * scaling, 0, 0) *
            local;
      }
      break;
    case BvhChannelTypes::Yposition:
      if (isRoot) {
        local =
            DirectX::XMMatrixTranslation(0, curve.values[frame] * scaling, 0) *
            local;
      }
      break;
    case BvhChannelTypes::Zposition:
      if (isRoot) {
        local =
            DirectX::XMMatrixTranslation(0, 0, curve.values[frame] * scaling) *
            local;
      }
      break;
    case BvhChannelTypes::Xrotation:
      local = DirectX::XMMatrixRotationX(
                  DirectX::XMConvertToRadians(curve.values[frame])) *
              local;
      break;
    case BvhChannelTypes::Yrotation:
      local = DirectX::XMMatrixRotationY(
                  DirectX::XMConvertToRadians(curve.values[frame])) *
              local;
      break;
    case BvhChannelTypes::Zrotation:
      local = DirectX::XMMatrixRotationZ(
                  DirectX::XMConvertToRadians(curve.values[frame])) *
              local;
      break;
    case BvhChannelTypes::None:
      goto BREAK;
    }
  }
BREAK:
  m = local * m;
  auto shape = DirectX::XMMatrixScaling(0.02f, 0.02f, 0.02f);
  DirectX::XMStoreFloat4x4((DirectX::XMFLOAT4X4 *)&*out, shape * m);
  ++out;
  for (auto &child : children) {
    child->ResolveFrame(frame, m, scaling, out);
  }
}

//
// BvhSolver
//
void BvhSolver::PushJoint(const BvhJoint &joint) {

  auto node = BvhNode::Create(joint);
  nodes_.push_back(node);
  instances_.push_back({});

  if (nodes_.size() == 1) {
    root_ = node;
    root_->isRoot = true;
  } else {
    auto parent = nodes_[joint.parent];
    parent->children.push_back(node);
  }
}

void BvhSolver::PushFrame(BvhTime time, std::span<const float> channelValues) {
  auto it = channelValues.begin();
  PushFrame(it, root_);
  assert(it == channelValues.end());
}

void BvhSolver::PushFrame(std::span<const float>::iterator &it,
                          std::shared_ptr<BvhNode> node) {

  for (int i = 0; i < node->ChannelCount(); ++i, ++it) {
    node->curves[i].values.push_back(*it);
  }
  for (auto &child : node->children) {
    PushFrame(it, child);
  }
}

std::span<cuber::Instance> BvhSolver::GetFrame(int frame) {
  auto span = std::span(instances_);
  auto it = span.begin();
  assert(scalingFactor_ > 0);
  root_->ResolveFrame(frame, DirectX::XMMatrixIdentity(), scalingFactor_, it);
  assert(it == span.end());
  return instances_;
}
