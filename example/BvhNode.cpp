#include "BvhNode.h"

//
// BvhNode
//
std::shared_ptr<BvhNode> BvhNode::Create(const BvhJoint &joint, float scaling,
                                         bool isRoot) {
  auto ptr = std::shared_ptr<BvhNode>(new BvhNode(joint.localOffset, isRoot));
  for (int i = 0; i < 6; ++i) {
    ptr->curves_[i].chnnel = joint.channels.values[i];
  }
  return ptr;
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
  auto shape = DirectX::XMMatrixScaling(1, 1, 1);
  DirectX::XMStoreFloat4x4((DirectX::XMFLOAT4X4 *)&*out, shape * m);
  ++out;
  for (auto &child : children_) {
    child->ResolveFrame(frame, m, out);
  }
}
