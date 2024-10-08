#include <DirectXMath.h>

#include "BvhNode.h"

const float DEFAULT_SIZE = 0.04f;

BvhNode::BvhNode(BvhJoint& joint)
  : joint_(joint)
{
  DirectX::XMStoreFloat4x4(
    (DirectX::XMFLOAT4X4*)&shape_,
    DirectX::XMMatrixScaling(DEFAULT_SIZE, DEFAULT_SIZE, DEFAULT_SIZE));
}

static DirectX::XMVECTOR
Float3(float x, float y, float z)
{
  DirectX::XMFLOAT3 v{ x, y, z };
  return DirectX::XMLoadFloat3(&v);
}
static float
Float3Len(DirectX::XMVECTOR v)
{
  DirectX::XMFLOAT3 l;
  DirectX::XMStoreFloat3(&l, DirectX::XMVector3Length(v));
  return l.x;
}
static DirectX::XMVECTOR
Float4(float x, float y, float z, float w)
{
  DirectX::XMFLOAT4 v{ x, y, z, w };
  return DirectX::XMLoadFloat4(&v);
}

void
BvhNode::CalcShape(float scaling)
{
  auto isRoot_ = joint_.index == 0;
  if (!isRoot_) {
    std::shared_ptr<BvhNode> tail;
    switch (children_.size()) {
      case 0:
        return;

      case 1:
        tail = children_.front();
        break;

      default:
        for (auto& child : children_) {
          if (!tail) {
            tail = child;
          } else if (std::abs(child->joint_.localOffset.x) <
                     std::abs(tail->joint_.localOffset.x)) {
            // coose center node
            tail = child;
          }
        }
    }

    auto Y = Float3(tail->joint_.localOffset.x * scaling,
                    tail->joint_.localOffset.y * scaling,
                    tail->joint_.localOffset.z * scaling);

    auto length = Float3Len(Y);
    // std::cout << name_ << "=>" << tail->name_ << "=" << length << std::endl;
    Y = DirectX::XMVector3Normalize(Y);
    auto Z = Float3(0, 0, 1);
    auto X = DirectX::XMVector3Cross(Y, Z);
    Z = DirectX::XMVector3Cross(X, Y);

    auto center = DirectX::XMMatrixTranslation(0, 0.5f, 0);
    auto scale = DirectX::XMMatrixScaling(DEFAULT_SIZE, length, DEFAULT_SIZE);
    auto r = DirectX::XMMATRIX(X, Y, Z, Float4(0, 0, 0, 1));

    auto shape = center * scale * r;
    DirectX::XMStoreFloat4x4((DirectX::XMFLOAT4X4*)&shape_, shape);
  }

  for (auto& child : children_) {
    child->CalcShape(scaling);
  }
}

// [x, y, z][c6][c5][c4][c3][c2][c1][parent][root]
void
BvhNode::ResolveFrame(const BvhFrame& frame,
                      const grapho::XMFLOAT4X4& __m,
                      float scaling,
                      std::span<grapho::XMFLOAT4X4>::iterator& out)
{
  auto [pos, r] = frame.Resolve(joint_.channels);
  auto rot = DirectX::XMLoadFloat4x4((const DirectX::XMFLOAT4X4*)&r);

  auto t = DirectX::XMMatrixTranslation(
    pos.x * scaling, pos.y * scaling, pos.z * scaling);
  auto local = rot * t;

  auto _m = local * DirectX::XMLoadFloat4x4((const DirectX::XMFLOAT4X4*)&__m);
  auto shape = DirectX::XMLoadFloat4x4((const DirectX::XMFLOAT4X4*)&shape_);
  DirectX::XMStoreFloat4x4((DirectX::XMFLOAT4X4*)&*out, shape * _m);
  ++out;

  grapho::XMFLOAT4X4 m;
  DirectX::XMStoreFloat4x4((DirectX::XMFLOAT4X4*)&m, _m);

  for (auto& child : children_) {
    child->ResolveFrame(frame, m, scaling, out);
  }
}
