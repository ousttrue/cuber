#pragma once
#include <chrono>
#include <optional>

class DxPlatform {
  struct DxPlatformImpl *impl_ = nullptr;

public:
  DxPlatform(const DxPlatform &) = delete;
  DxPlatform &operator=(const DxPlatform &) = delete;
  DxPlatform();
  ~DxPlatform();
  bool Create();
  std::optional<std::chrono::milliseconds> NewFrame(const float clear_color[4]);
  void EndFrame(struct ImDrawData *data);
};
