#pragma once
#include <chrono>
#include <optional>

// using GlfwTime = std::chrono::duration<float, std::ratio<1, 1>>;

class DxPlatform {
  struct DxPlatformImpl *impl_ = nullptr;

public:
  DxPlatform(const DxPlatform &) = delete;
  DxPlatform &operator=(const DxPlatform &) = delete;
  DxPlatform();
  ~DxPlatform();
  bool Create();
  bool NewFrame(const float clear_color[4]);
  void EndFrame(struct ImDrawData *data);
};
