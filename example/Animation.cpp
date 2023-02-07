#include "Animation.h"
#include "Bvh.h"
#include "BvhSolver.h"
#include <asio.hpp>
#include <fstream>
#include <iostream>
#include <vector>

struct AnimationImpl {
  asio::io_context &io_;
  std::shared_ptr<asio::steady_timer> timer_;
  std::chrono::steady_clock::time_point startTime_;

  std::shared_ptr<Bvh> bvh_;
  BvhSolver bvhSolver_;
  std::list<Animation::OnFrameFunc> onFrameCallbacks_;

  int counter_ = 0;
  int lastFrame_ = -1;

  AnimationImpl(asio::io_context &io) : io_(io) {}

  void Stop() {
    timer_->cancel();
    timer_.reset();
  }

  void BeginTimer(std::chrono::nanoseconds interval) {
    startTime_ = std::chrono::steady_clock::now();
    timer_ = std::shared_ptr<asio::steady_timer>(
        new asio::steady_timer(io_));
    AsyncWait(interval);
  }

  void AsyncWait(std::chrono::nanoseconds interval) {
    if (auto timer = timer_) {
      try {
        timer->expires_after(interval);        
        timer->async_wait([self = this, interval](const std::error_code &) {
          self->Update();
          self->AsyncWait(interval);
        });
      } catch (std::exception const &e) {
        std::cout << "AsyncWait catch: " << e.what() << std::endl;
      }
    }
  }

  void Update() {
    ++counter_;
    auto elapsed = std::chrono::steady_clock::now() - startTime_;
    auto index = bvh_->TimeToIndex(elapsed);
    if (index == lastFrame_) {
      std::cout << index << std::endl;
      return;
    }
    lastFrame_ = index;
    for (auto &callback : onFrameCallbacks_) {
      callback(elapsed, bvhSolver_.GetFrame(index));
    }
  }

  std::shared_ptr<Bvh> Load(std::string_view file) {

    bvh_ = Bvh::ParseFile(file);
    if (!bvh_) {
      return {};
    }

    auto scalingFactor = bvh_->GuessScaling();

    bvhSolver_.Initialize();
    for (auto &joint : bvh_->joints) {
      bvhSolver_.PushJoint(joint, scalingFactor);
    };
    bvhSolver_.CalcShape();

    int frameCount = bvh_->FrameCount();
    for (int i = 0; i < frameCount; ++i) {
      auto frame = bvh_->GetFrame(i);
      // auto time = bvh_->frame_time * i;
      bvhSolver_.PushFrame(frame, scalingFactor);
    }

    BeginTimer(
        std::chrono::duration_cast<std::chrono::nanoseconds>(bvh_->frame_time));

    return bvh_;
  }
};

Animation::Animation(asio::io_context &io) : impl_(new AnimationImpl(io)) {}
Animation::~Animation() { delete (impl_); }
std::shared_ptr<Bvh> Animation::Load(std::string_view file) {
  return impl_->Load(file);
}
void Animation::OnFrame(const OnFrameFunc &onFrame) {
  impl_->onFrameCallbacks_.push_back(onFrame);
}
void Animation::Stop() { impl_->Stop(); }
