#pragma once

class GuiWindow {
  struct GLFWwindow *window_ = nullptr;

public:
  GuiWindow();
  ~GuiWindow();
  struct GLFWwindow *Create();
  bool NewFrame(int *display_w, int *display_h);
  void EndFrame();
  const char *GlslVersion() const;
};
