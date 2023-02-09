// Dear ImGui: standalone example application for DirectX 11
// If you are new to Dear ImGui, read documentation from the docs/ folder + read
// the top of imgui.cpp. Read online:
// https://github.com/ocornut/imgui/tree/master/docs

#include "DxPlatform.h"
#include "GuiApp.h"
#include <imgui.h>

// Main code
int main(int, char **) {
  // imgui
  GuiApp app;

  DxPlatform platform;
  if (!platform.Create()) {
    return 1;
  }

  // main loop
  while (auto time = platform.NewFrame(app.clear_color)) {
    // imgui
    {
      app.UpdateGui();
      // bvhPanel.UpdateGui();
    }
    auto data = app.RenderGui();

    // scene
    {
      // renderer.RenderScene(*time, app.projection, app.view);
      platform.EndFrame(data);
    }
  }

  return 0;
}
