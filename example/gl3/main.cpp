#include "BvhPanel.h"
#include "GlRenderer.h"
#include "GuiApp.h"
#include "GuiWindow.h"

int main(int argc, char **argv) {
  // window
  GuiWindow gui;
  auto window = gui.Create();
  if (!window) {
    return 1;
  }

  // imgui
  GuiApp app(gui.GlslVersion());
  gui.InitGuiPlatform(window);

  // OpenGL
  GlRenderer renderer;

  {
    // bvh scene
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
    while (auto time = gui.NewFrame(app.clear_color)) {
      // imgui
      {
        app.UpdateGui();
        bvhPanel.UpdateGui();
      }
      auto data = app.RenderGui();

      // scene
      {
        renderer.RenderScene(*time, app.projection, app.view);
        gui.EndFrame(data);
      }
    }

    gui.ShutdownGuiPlatform();
  }

  return 0;
}
