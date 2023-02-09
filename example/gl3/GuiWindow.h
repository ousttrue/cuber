#pragma once
#include <chrono>
#include <optional>

using GlfwTime = std::chrono::duration<float, std::ratio<1, 1>>;

class GuiWindow {
  struct GLFWwindow *window_ = nullptr;

public:
  GuiWindow();
  ~GuiWindow();
  GuiWindow(const GuiWindow &) = delete;
  GuiWindow &operator=(const GuiWindow &) = delete;
  struct GLFWwindow *Create();
  void InitGuiPlatform(GLFWwindow *window);
  void ShutdownGuiPlatform();
  std::optional<GlfwTime> NewFrame(const float clear_color[4]);
  void EndFrame(struct ImDrawData *data);
  const char *GlslVersion() const;
};
