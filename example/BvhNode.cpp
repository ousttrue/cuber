#include "BvhNode.h"

BvhNode::BvhNode(const DirectX::XMFLOAT3 &offset, bool isRoot)
    : isRoot_(isRoot), localOffset_(offset) {
  DirectX::XMStoreFloat4x4(&shape_, DirectX::XMMatrixScaling(0.2f, 0.2f, 0.2f));
}

std::shared_ptr<BvhNode> BvhNode::Create(const BvhJoint &joint, float scaling,
                                         bool isRoot) {
  auto offset = DirectX::XMFLOAT3(joint.localOffset.x * scaling,
                                  joint.localOffset.y * scaling,
                                  joint.localOffset.z * scaling);
  auto ptr = std::shared_ptr<BvhNode>(new BvhNode(offset, isRoot));
  for (int i = 0; i < 6; ++i) {
    ptr->curves_[i].chnnel = joint.channels.values[i];
  }
  return ptr;
}

void BvhNode::PushFrame(std::span<const float>::iterator &it, float scaling) {

  for (int i = 0; i < ChannelCount(); ++i, ++it) {
    auto &curve = curves_[i];
    switch (curve.chnnel) {
    case BvhChannelTypes::Xposition:
    case BvhChannelTypes::Yposition:
    case BvhChannelTypes::Zposition:
      curve.values.push_back(*it * scaling);
      break;
    case BvhChannelTypes::Xrotation:
    case BvhChannelTypes::Yrotation:
    case BvhChannelTypes::Zrotation:
      curve.values.push_back(*it);
      break;

    default:
      break;
    }
  }
  for (auto &child : children_) {
    child->PushFrame(it, scaling);
  }
}

// [x, y, z][c6][c5][c4][c3][c2][c1][parent][root]
void BvhNode::ResolveFrame(int frame, DirectX::XMMATRIX m,
                           std::span<cuber::Instance>::iterator &out) {

  auto local = isRoot_ ? DirectX::XMMatrixIdentity()
                       : DirectX::XMMatrixTranslation(
                             localOffset_.x, localOffset_.y, localOffset_.z);
  for (int i = 0; i < 6; ++i) {
    auto &curve = curves_[i];
    switch (curve.chnnel) {
    case BvhChannelTypes::Xposition:
      if (isRoot_) {
        local = DirectX::XMMatrixTranslation(curve.values[frame], 0, 0) * local;
      }
      break;
    case BvhChannelTypes::Yposition:
      if (isRoot_) {
        local = DirectX::XMMatrixTranslation(0, curve.values[frame], 0) * local;
      }
      break;
    case BvhChannelTypes::Zposition:
      if (isRoot_) {
        local = DirectX::XMMatrixTranslation(0, 0, curve.values[frame]) * local;
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
  for (auto &child : children_) {
    child->ResolveFrame(frame, m, out);
  }
}
