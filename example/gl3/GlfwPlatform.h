#pragma once
#include <chrono>
#include <optional>

using GlfwTime = std::chrono::duration<float, std::ratio<1, 1>>;

class GlfwPlatform {
  struct GLFWwindow *window_ = nullptr;

public:
  GlfwPlatform(const GlfwPlatform &) = delete;
  GlfwPlatform &operator=(const GlfwPlatform &) = delete;
  GlfwPlatform();
  ~GlfwPlatform();
  const char *GlslVersion() const;
  struct GLFWwindow *Create();
  std::optional<GlfwTime> NewFrame(const float clear_color[4]);
  void EndFrame(struct ImDrawData *data);
};
