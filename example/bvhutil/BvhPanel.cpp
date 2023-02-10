#include "BvhPanel.h"
#include "Animation.h"
#include "Bvh.h"
#include "BvhSolver.h"
#include "UdpSender.h"
#include <asio.hpp>
#include <imgui.h>
#include <thread>

class BvhPanelImpl {
  asio::io_context io_;
  asio::executor_work_guard<asio::io_context::executor_type> work_;
  Animation animation_;
  UdpSender sender_;
  std::thread thread_;
  std::shared_ptr<Bvh> bvh_;
  asio::ip::udp::endpoint ep_;
  bool enablePackQuat_ = false;
  std::vector<int> parentMap_;

  std::vector<DirectX::XMFLOAT4X4> instancies_;
  std::mutex mutex_;
  BvhSolver bvhSolver_;

public:
  BvhPanelImpl()
      : work_(asio::make_work_guard(io_)), animation_(io_), sender_(io_),
        ep_(asio::ip::address::from_string("127.0.0.1"), 54345) {
    animation_.OnFrame([self = this](const BvhFrame &frame) {
      self->sender_.SendFrame(self->ep_, self->bvh_, frame,
                              self->enablePackQuat_);
    });
    thread_ = std::thread([self = this]() {
      try {
        self->io_.run();
        std::cout << "[asio] end" << std::endl;
      } catch (std::exception const &e) {
        std::cout << "[asio] catch" << e.what() << std::endl;
      }
    });

    // bind bvh animation to renderer
    animation_.OnFrame(
        [self = this](const BvhFrame &frame) { self->SyncFrame(frame); });
  }

  ~BvhPanelImpl() {
    animation_.Stop();
    work_.reset();
    thread_.join();
  }

  void SetBvh(const std::shared_ptr<Bvh> &bvh) {
    bvh_ = bvh;
    if (!bvh_) {
      return;
    }
    animation_.SetBvh(bvh);
    for (auto &joint : bvh_->joints) {
      parentMap_.push_back(joint.parent);
    }
    sender_.SendSkeleton(ep_, bvh_);

    bvhSolver_.Initialize(bvh);
  }

  void UpdateGui() {
    if (!bvh_) {
      return;
    }
    // bvh panel
    ImGui::Begin("BVH");

    ImGui::LabelText("bvh", "%zu joints", bvh_->joints.size());

    ImGui::Checkbox("use quaternion pack32", &enablePackQuat_);

    if (ImGui::Button("send skeleton")) {
      sender_.SendSkeleton(ep_, bvh_);
    }

    ImGui::End();
  }

  void SyncFrame(const BvhFrame &frame) {
    auto instances = bvhSolver_.ResolveFrame(frame);
    {
      std::lock_guard<std::mutex> lock(mutex_);
      instancies_.assign(instances.begin(), instances.end());
    }
  }

  std::span<const DirectX::XMFLOAT4X4> GetCubes() {
    std::lock_guard<std::mutex> lock(mutex_);
    return instancies_;
  }
};

BvhPanel::BvhPanel() : impl_(new BvhPanelImpl) {}
BvhPanel::~BvhPanel() { delete impl_; }
void BvhPanel::SetBvh(const std::shared_ptr<Bvh> &bvh) { impl_->SetBvh(bvh); }
void BvhPanel::UpdateGui() { impl_->UpdateGui(); }
std::span<const DirectX::XMFLOAT4X4> BvhPanel::GetCubes() {
  return impl_->GetCubes();
}
