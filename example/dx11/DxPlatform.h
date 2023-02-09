#pragma once
#include <chrono>
#include <optional>
#include <winrt/base.h>
#include <d3d11.h>

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
  winrt::com_ptr<ID3D11Device> GetDevice() const;
};
