#include "UdpSender.h"
#include "../subprojects/meshutils/Source/MeshUtils/muQuat32.h"
#include "Bvh.h"
#include "Payload.h"
#include <iostream>

UdpSender::UdpSender(asio::io_context &io)
    : socket_(io, asio::ip::udp::endpoint(asio::ip::udp::v4(), 0)) {}

std::shared_ptr<Payload> UdpSender::GetOrCreatePayload() {
  std::lock_guard<std::mutex> lock(mutex_);
  if (payloads_.empty()) {
    return std::make_shared<Payload>();
  } else {
    auto payload = payloads_.front();
    payloads_.pop_front();
    return payload;
  }
}

void UdpSender::ReleasePayload(const std::shared_ptr<Payload> &payload) {
  std::lock_guard<std::mutex> lock(mutex_);
  payloads_.push_back(payload);
}

void UdpSender::SendSkeleton(asio::ip::udp::endpoint ep,
                             const std::shared_ptr<Bvh> &bvh) {
  auto payload = GetOrCreatePayload();
  joints_.clear();
  auto scaling = bvh->GuessScaling();
  for (auto joint : bvh->joints) {
    joints_.push_back({
        .parentBoneIndex = joint.parent,
        .boneType = 0,
        .xFromParent = joint.localOffset.x * scaling,
        .yFromParent = joint.localOffset.y * scaling,
        .zFromParent = joint.localOffset.z * scaling,
    });
  }
  payload->SetSkeleton(joints_);

  socket_.async_send_to(asio::buffer(payload->buffer), ep,
                        [self = this, payload](asio::error_code ec,
                                               std::size_t bytes_transferred) {
                          self->ReleasePayload(payload);
                        });
}

void UdpSender::SendFrame(asio::ip::udp::endpoint ep,
                          const std::shared_ptr<Bvh> &bvh,
                          const BvhFrame &frame, bool pack) {

  auto payload = GetOrCreatePayload();

  //   auto &hips = instances[0];
  //   payload->SetFrame(time, hips._41, hips._42, hips._43, pack);

  //   for (int i = 0; i < instances.size(); ++i) {
  //     auto &instance = instances[i];
  //     auto parentIndex = parentMap[i];
  //     auto m = DirectX::XMLoadFloat4x4(&instance);
  //     // auto r = DirectX::XMQuaternionRotationMatrix(m);
  //     auto parent = DirectX::XMMatrixIdentity();
  //     if (parentIndex != 65535) {
  //       parent = DirectX::XMLoadFloat4x4(&instances[parentIndex]);
  //     }
  //     auto localRotation = DirectX::XMQuaternionRotationMatrix(
  //         m * DirectX::XMMatrixInverse(nullptr, parent));

  //     // DirectX::XMVECTOR axis;
  //     // float angle;
  //     // DirectX::XMQuaternionToAxisAngle(&axis, &angle, local);
  //     // local = DirectX::XMQuaternionRotationAxis(axis, angle*0.5f);

  //     DirectX::XMFLOAT4 rotation;
  //     DirectX::XMStoreFloat4(&rotation, localRotation);

  //     if (pack) {
  //       auto packed =
  //           quat_packer::Pack(rotation.x, rotation.y, rotation.z,
  //           rotation.w);
  // #if _DEBUG
  //       {
  //         mu::quatf debug_q{rotation.x, rotation.y, rotation.z, rotation.w};
  //         auto debug_packed = mu::quat32(debug_q);
  //         assert(*(uint32_t *)&debug_packed.value == packed);
  //       }
  //       payload->Push(packed);
  // #endif
  //     } else {
  //       payload->Push(rotation);
  //     }
  //   }

  //   socket_.async_send_to(asio::buffer(payload->buffer), ep,
  //                         [self = this, payload](asio::error_code ec,
  //                                                std::size_t
  //                                                bytes_transferred) {
  //                           self->ReleasePayload(payload);
  //                         });

  ReleasePayload(payload);
}
