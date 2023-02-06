#include "Animation.h"
#include "Bvh.h"
#include "BvhSolver.h"
#include <asio.hpp>
#include <fstream>
#include <iostream>
#include <vector>

template <typename T>
static std::vector<T> ReadAllBytes(const std::string &filename) {
  std::ifstream ifs(filename.c_str(), std::ios::binary | std::ios::ate);
  if (!ifs) {
    return {};
  }
  auto pos = ifs.tellg();
  auto size = pos / sizeof(T);
  if (pos % sizeof(T)) {
    ++size;
  }
  std::vector<T> buffer(size);
  ifs.seekg(0, std::ios::beg);
  ifs.read(buffer.data(), pos);
  return buffer;
}

struct AnimationImpl {
  asio::io_context &io_;
  std::shared_ptr<asio::steady_timer> timer_;
  std::chrono::steady_clock::time_point startTime_;

  Bvh bvh_;
  BvhSolver bvhSolver_;
  std::list<Animation::OnFrameFunc> onFrameCallbacks_;

  AnimationImpl(asio::io_context &io) : io_(io) {}

  void Stop() {
    timer_->cancel();
    timer_.reset();
  }

  void BeginTimer(std::chrono::nanoseconds interval) {
    startTime_ = std::chrono::steady_clock::now();
    timer_ = std::shared_ptr<asio::steady_timer>(
        new asio::steady_timer(io_, interval));
    AsyncWait();
  }

  void AsyncWait() {
    if (auto timer = timer_) {
      try {
        timer->async_wait([self = this](const std::error_code &) {
          self->Update();
          self->AsyncWait();
        });
      } catch (std::exception const &e) {
        std::cout << "AsyncWait catch: " << e.what() << std::endl;
      }
    }
  }

  void Update() {
    auto elapsed = std::chrono::steady_clock::now() - startTime_;
    auto index = bvh_.TimeToIndex(elapsed);
    for (auto &callback : onFrameCallbacks_) {
      callback(elapsed, bvhSolver_.GetFrame(index));
    }
  }

  bool Load(std::string_view file) {

    auto bytes = ReadAllBytes<char>(std::string(file.begin(), file.end()));
    if (bytes.empty()) {
      return false;
    }
    std::cout << "load: " << file << " " << bytes.size() << "bytes"
              << std::endl;
    if (!bvh_.Parse({bytes.begin(), bytes.end()})) {
      return false;
    }
    std::cout << bvh_ << std::endl;

    // guess bvh scale
    float scalingFactor = 1.0f;
    if (bvh_.max_height < 2) {
      // maybe meter scale. do nothing
    } else if (bvh_.max_height < 200) {
      // maybe cm scale
      scalingFactor = 0.01f;
    }

    bvhSolver_.Initialize();
    for (auto &joint : bvh_.joints) {
      bvhSolver_.PushJoint(joint, scalingFactor);
    };
    bvhSolver_.CalcShape();

    int frameCount = bvh_.FrameCount();
    for (int i = 0; i < frameCount; ++i) {
      auto frame = bvh_.GetFrame(i);
      auto time = bvh_.frame_time * i;
      bvhSolver_.PushFrame(time, frame, scalingFactor);
    }

    BeginTimer(
        std::chrono::duration_cast<std::chrono::nanoseconds>(bvh_.frame_time));

    return true;
  }
};

Animation::Animation(asio::io_context &io) : impl_(new AnimationImpl(io)) {}
Animation::~Animation() { delete (impl_); }
void Animation::Load(std::string_view file) { impl_->Load(file); }
void Animation::OnFrame(const OnFrameFunc &onFrame) {
  impl_->onFrameCallbacks_.push_back(onFrame);
}
void Animation::Stop() { impl_->Stop(); }
