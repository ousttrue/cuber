#include "BvhPanel.h"
#include "GlRenderer.h"
#include "GlfwPlatform.h"
#include "GuiApp.h"

int main(int argc, char **argv) {
  GlfwPlatform platform;

  // imgui
  GuiApp app(platform.GlslVersion());

  // window
  auto window = platform.Create();
  if (!window) {
    return 1;
  }

  {
    // bvh scene
    GlRenderer renderer;
    BvhPanel bvhPanel;

    // bind bvh animation to renderer
    bvhPanel.OnFrame(
        [&renderer](const BvhFrame &frame) { renderer.SetFrame(frame); });

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

    platform.ShutdownGuiBackend();
  }

  return 0;
}
