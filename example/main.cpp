#include "../subprojects/meshutils/Source/MeshUtils/muQuat32.h"
#include "Animation.h"
#include "Bvh.h"
#include "GlRenderer.h"
#include "GuiApp.h"
#include "GuiWindow.h"
#include "TurnTable.h"
#include "quat_packer.h"
#include <DirectXMath.h>
#include <asio.hpp>
#include <iostream>
#include <thread>

// must after asio.hpp
#include <Windows.h>

// must after Windows.h
#include <GL/GL.h>

#include "srht.h"

struct Payload {
  std::vector<uint8_t> buffer;

  Payload(const Payload &) = delete;
  Payload &operator=(const Payload &) = delete;
  Payload() { std::cout << "Payload::Payload" << std::endl; }
  ~Payload() { std::cout << "Payload::~Payload" << std::endl; }

  void Push(const void *begin, const void *end) {
    auto dst = buffer.size();
    auto size = std::distance((const char *)begin, (const char *)end);
    buffer.resize(dst + size);
    std::copy((const char *)begin, (const char *)end, buffer.data() + dst);
  }

  void SetSkeleton(std::span<srht::JointDefinition> joints) {
    buffer.clear();

    srht::SkeletonHeader header{
        .magic = {},
        .skeletonId = 0,
        .jointCount = 27,
        .hasInitialRotation = 0,
    };
    Push((const char *)&header, (const char *)&header + sizeof(header));
    Push(joints.data(), joints.data() + joints.size());
  }

  void SetFrame(std::chrono::nanoseconds time, float x, float y, float z,
                std::span<srht::PackQuat> rotations) {
    buffer.clear();

    srht::FrameHeader header{
        .magic = {},
        .time = time.count(),
        .skeletonId = 0,
        .x = x,
        .y = y,
        .z = z,
    };
    Push((const char *)&header, (const char *)&header + sizeof(header));
    Push(rotations.data(), rotations.data() + rotations.size());
  }
};

class UdpSender {
  asio::ip::udp::socket socket_;
  std::list<std::shared_ptr<Payload>> payloads_;
  std::mutex mutex_;
  std::vector<srht::PackQuat> rotations_;
  std::vector<srht::JointDefinition> joints_;

public:
  UdpSender(asio::io_context &io)
      : socket_(io, asio::ip::udp::endpoint(asio::ip::udp::v4(), 0)) {}

  std::shared_ptr<Payload> GetOrCreatePayload() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (payloads_.empty()) {
      return std::make_shared<Payload>();
    } else {
      auto payload = payloads_.front();
      payloads_.pop_front();
      return payload;
    }
  }

  void ReleasePayload(const std::shared_ptr<Payload> &payload) {
    std::lock_guard<std::mutex> lock(mutex_);
    payloads_.push_back(payload);
  }

  void SendSkeleton(asio::ip::udp::endpoint ep,
                    const std::shared_ptr<Bvh> &bvh) {
    auto payload = GetOrCreatePayload();
    joints_.clear();
    for (auto joint : bvh->joints) {
      joints_.push_back({
          .parentBoneIndex = joint.parent,
          .boneType = 0,
          .xFromParent = joint.localOffset.x,
          .yFromParent = joint.localOffset.y,
          .zFromParent = joint.localOffset.z,
      });
    }
    payload->SetSkeleton(joints_);

    socket_.async_send_to(
        asio::buffer(payload->buffer), ep,
        [self = this, payload](asio::error_code ec,
                               std::size_t bytes_transferred) {
          self->ReleasePayload(payload);
        });
  }

  void SendFrame(asio::ip::udp::endpoint ep, std::chrono::nanoseconds time,
                 std::span<DirectX::XMFLOAT4X4> instances) {

    auto payload = GetOrCreatePayload();

    rotations_.clear();
    for (auto &instance : instances) {
      auto m = DirectX::XMLoadFloat4x4(&instance);
      auto r = DirectX::XMQuaternionRotationMatrix(m);
      DirectX::XMFLOAT4 rotation;
      DirectX::XMStoreFloat4(&rotation, r);
      auto packed =
          quat_packer::pack(rotation.x, rotation.y, rotation.z, rotation.w);
#if _DEBUG
      {
        mu::quatf debug_q{rotation.x, rotation.y, rotation.z, rotation.w};
        auto debug_packed = mu::quat32(debug_q);
        assert(*(uint32_t *)&debug_packed.value == packed);
      }
#endif
      rotations_.push_back({.value = packed});
    }

    auto &hips = instances[0];

    payload->SetFrame(time, hips._41, hips._42, hips._43, rotations_);

    socket_.async_send_to(
        asio::buffer(payload->buffer), ep,
        [self = this, payload](asio::error_code ec,
                               std::size_t bytes_transferred) {
          self->ReleasePayload(payload);
        });
  }
};

int main(int argc, char **argv) {
  asio::io_context io;

  GuiWindow gui;
  auto window = gui.Create();
  if (!window) {
    return 1;
  }

  GuiApp app(window, gui.GlslVersion());

  GlRenderer renderer;

  Animation animation(io);
  animation.OnFrame([&renderer](auto time, auto instances) {
    renderer.SetInstances(instances);
  });

  asio::ip::udp::endpoint ep(asio::ip::address::from_string("127.0.0.1"),
                             54345);
  UdpSender sender(io);
  animation.OnFrame([&sender, &ep](auto time, auto instances) {
    sender.SendFrame(ep, time, instances);
  });

  if (argc > 1) {
    if (auto bvh = animation.Load(argv[1])) {
      sender.SendSkeleton(ep, bvh);
    }
  }

  // start
  auto work = asio::make_work_guard(io);
  std::thread thread([&io]() {
    try {
      io.run();
      std::cout << "[asio] end" << std::endl;
    } catch (std::exception const &e) {
      std::cout << "[asio] catch" << e.what() << std::endl;
    }
  });

  // main loop
  int display_w, display_h;
  while (auto time = gui.NewFrame(&display_w, &display_h)) {
    app.UpdateGui();

    // render
    glViewport(0, 0, display_w, display_h);
    glClearColor(app.clear_color[0], app.clear_color[1], app.clear_color[2],
                 app.clear_color[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // scene
    renderer.RenderScene(*time, app.projection, app.view);

    // app
    app.RenderGui();

    gui.EndFrame();
  }

  animation.Stop();
  work.reset();
  thread.join();

  return 0;
}
