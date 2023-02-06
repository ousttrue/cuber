#pragma once
#include "TurnTable.h"

class GuiApp {
  // Our state
  bool show_demo_window = true;
  bool show_another_window = false;
  float clear_color_[4] = {0.45f, 0.55f, 0.60f, 1.00f};
  TurnTable turntable_ = {};
  
public:
  float clear_color[4] = {};
  float projection[16] = {
      1, 0, 0, 0, //
      0, 1, 0, 0, //
      0, 0, 1, 0, //
      0, 0, 0, 1, //
  };
  float view[16] = {
      1, 0, 0, 0, //
      0, 1, 0, 0, //
      0, 0, 1, 0, //
      0, 0, 0, 1, //
  };
  GuiApp(const GuiApp &) = delete;
  GuiApp &operator=(const GuiApp &) = delete;
  GuiApp(struct GLFWwindow *window, const char *glsl_version);
  ~GuiApp();
  void UpdateGui();
  void RenderGui();

private:
  void UpdateGuiDockspace();
};
