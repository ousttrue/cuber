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
  std::optional<GlfwTime> NewFrame(int *display_w, int *display_h);
  void EndFrame();
  const char *GlslVersion() const;
};
