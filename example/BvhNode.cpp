#include "BvhNode.h"

const float DEFAULT_SIZE = 0.04f;

BvhNode::BvhNode(std::string_view name, const DirectX::XMFLOAT3 &offset,
                 bool isRoot)
    : name_(name), isRoot_(isRoot), localOffset_(offset) {
  DirectX::XMStoreFloat4x4(
      &shape_,
      DirectX::XMMatrixScaling(DEFAULT_SIZE, DEFAULT_SIZE, DEFAULT_SIZE));
}

std::shared_ptr<BvhNode> BvhNode::Create(const BvhJoint &joint, float scaling,
                                         bool isRoot) {
  auto offset = DirectX::XMFLOAT3(joint.localOffset.x * scaling,
                                  joint.localOffset.y * scaling,
                                  joint.localOffset.z * scaling);
  auto ptr = std::shared_ptr<BvhNode>(new BvhNode(joint.name, offset, isRoot));
  for (int i = 0; i < 6; ++i) {
    ptr->curves_[i].chnnel = joint.channels.values[i];
  }
  return ptr;
}

void BvhNode::CalcShape() {
  if (!isRoot_) {
    std::shared_ptr<BvhNode> tail;
    switch (children_.size()) {
    case 0:
      return;

    case 1:
      tail = children_.front();
      break;

    default:
      for (auto &child : children_) {
        if (!tail) {
          tail = child;
        } else if (std::abs(child->localOffset_.x) <
                   std::abs(tail->localOffset_.x)) {
          // coose center node
          tail = child;
        }
      }
    }

    auto Y = tail->localOffset_;
    auto length = Y.Length();
    // std::cout << name_ << "=>" << tail->name_ << "=" << length << std::endl;
    Y.Normalize();
    DirectX::SimpleMath::Vector3 Z(0, 0, 1);
    auto X = Y.Cross(Z);
    Z = X.Cross(Y);

    auto center = DirectX::SimpleMath::Matrix::CreateTranslation(0, 0.5f, 0);
    auto scale = DirectX::SimpleMath::Matrix::CreateScale(DEFAULT_SIZE, length,
                                                          DEFAULT_SIZE);
    auto r = DirectX::SimpleMath::Matrix(X, Y, Z);

    auto shape = center * scale * r;
    DirectX::XMStoreFloat4x4(&shape_, shape);
  }

  for (auto &child : children_) {
    child->CalcShape();
  }
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
void BvhNode::ResolveFrame(const BvhFrame &frame, DirectX::XMMATRIX m,
                           std::span<DirectX::XMFLOAT4X4>::iterator &out) {

  auto local = isRoot_ ? DirectX::XMMatrixIdentity()
                       : DirectX::XMMatrixTranslation(
                             localOffset_.x, localOffset_.y, localOffset_.z);  
  
  for (int i = 0; i < 6; ++i) {
    auto &curve = curves_[i];
    switch (curve.chnnel) {
    case BvhChannelTypes::Xposition:
      if (isRoot_) {
        local = DirectX::XMMatrixTranslation(curve.values[frame.index], 0, 0) * local;
      }
      break;
    case BvhChannelTypes::Yposition:
      if (isRoot_) {
        local = DirectX::XMMatrixTranslation(0, curve.values[frame.index], 0) * local;
      }
      break;
    case BvhChannelTypes::Zposition:
      if (isRoot_) {
        local = DirectX::XMMatrixTranslation(0, 0, curve.values[frame.index]) * local;
      }
      break;
    case BvhChannelTypes::Xrotation:
      local = DirectX::XMMatrixRotationX(
                  DirectX::XMConvertToRadians(curve.values[frame.index])) *
              local;
      break;
    case BvhChannelTypes::Yrotation:
      local = DirectX::XMMatrixRotationY(
                  DirectX::XMConvertToRadians(curve.values[frame.index])) *
              local;
      break;
    case BvhChannelTypes::Zrotation:
      local = DirectX::XMMatrixRotationZ(
                  DirectX::XMConvertToRadians(curve.values[frame.index])) *
              local;
      break;
    case BvhChannelTypes::None:
      goto BREAK;
    }
  }
BREAK:
  m = local * m;
  auto shape = DirectX::XMLoadFloat4x4(&shape_);
  DirectX::XMStoreFloat4x4(&*out, shape * m);
  ++out;
  for (auto &child : children_) {
    child->ResolveFrame(frame, m, out);
  }
}
