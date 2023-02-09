#include "BvhPanel.h"
#include "DxPlatform.h"
#include "DxRenderer.h"
#include "GuiApp.h"

// Main code
int main(int argc, char **argv) {
  // imgui
  GuiApp app;

  DxPlatform platform;
  if (!platform.Create()) {
    return 1;
  }

  // bvh scene
  DxRenderer renderer;
  BvhPanel bvhPanel;

  // bind bvh animation to renderer
  bvhPanel.OnFrame(
      [&renderer](const BvhFrame &frame) { renderer.SyncFrame(frame); });

  // load bvh
  if (argc > 1) {
    if (auto bvh = Bvh::ParseFile(argv[1])) {
      bvhPanel.SetBvh(bvh);
      renderer.SetBvh(bvh);
    }
  }

  // main loop
  while (auto time = platform.NewFrame(app.clear_color)) {
    // imgui
    {
      app.UpdateGui();
      bvhPanel.UpdateGui();
    }
    auto data = app.RenderGui();

    // scene
    {
      renderer.RenderScene(*time, app.projection, app.view);
      platform.EndFrame(data);
    }
  }

  return 0;
}
