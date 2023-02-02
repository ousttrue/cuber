#pragma once

class GuiApp {
  // Our state
  bool show_demo_window = true;
  bool show_another_window = false;
  float clear_color_[4] = {0.45f, 0.55f, 0.60f, 1.00f};

public:
  float clear_color[4] = {};
  GuiApp(const GuiApp &) = delete;
  GuiApp &operator=(const GuiApp &) = delete;
  GuiApp(struct GLFWwindow *window, const char *glsl_version);
  ~GuiApp();
  void UpdateGui();
  void RenderGui();
};
