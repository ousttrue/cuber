#include "BvhPanel.h"
#include "Animation.h"
#include "Bvh.h"
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

public:
  BvhPanelImpl()
      : work_(asio::make_work_guard(io_)), animation_(io_), sender_(io_),
        ep_(asio::ip::address::from_string("127.0.0.1"), 54345) {
    animation_.OnFrame([self = this](auto time, auto instances) {
      self->sender_.SendFrame(self->ep_, time, instances, self->parentMap_,
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
  void OnFrame(const Animation::OnFrameFunc &onFrame) {
    animation_.OnFrame(onFrame);
  }
};

BvhPanel::BvhPanel() : impl_(new BvhPanelImpl) {}
BvhPanel::~BvhPanel() { delete impl_; }
void BvhPanel::SetBvh(const std::shared_ptr<Bvh> &bvh) { impl_->SetBvh(bvh); }
void BvhPanel::OnFrame(const Animation::OnFrameFunc &onFrame) {
  impl_->OnFrame(onFrame);
}
void BvhPanel::UpdateGui() { impl_->UpdateGui(); }
