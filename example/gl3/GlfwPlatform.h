#pragma once
#include <chrono>
#include <optional>

using GlfwTime = std::chrono::duration<float, std::ratio<1, 1>>;

class GlfwPlatform {
  struct GLFWwindow *window_ = nullptr;

public:
  GlfwPlatform();
  ~GlfwPlatform();
  GlfwPlatform(const GlfwPlatform &) = delete;
  GlfwPlatform &operator=(const GlfwPlatform &) = delete;
  struct GLFWwindow *Create();
  void InitGui(GLFWwindow *window);
  void ShutdownGuiPlatform();
  std::optional<GlfwTime> NewFrame(const float clear_color[4]);
  void EndFrame(struct ImDrawData *data);
  const char *GlslVersion() const;
};
