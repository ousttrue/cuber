#pragma once
#include <DirectXMath.h>
#include <functional>
#include <span>
#include <string_view>

namespace asio {
class io_context;
} // namespace asio

class Animation {
  struct AnimationImpl *impl_ = nullptr;

public:
  Animation(asio::io_context &io);
  ~Animation();
  void Load(std::string_view file);
  void
  OnFrame(const std::function<void(std::span<DirectX::XMFLOAT4X4>)> &onFrame);
  void Stop();
};
