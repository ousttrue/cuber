#include "BvhPanel.h"
#include "DxPlatform.h"
#include "GuiApp.h"
#include <cuber/DxCubeRenderer.h>

// Main code
int main(int argc, char **argv) {
  // imgui
  GuiApp app(true);

  DxPlatform platform;
  if (!platform.Create()) {
    return 1;
  }

  // bvh scene
  BvhPanel bvhPanel;

  // load bvh
  if (argc > 1) {
    if (auto bvh = Bvh::ParseFile(argv[1])) {
      bvhPanel.SetBvh(bvh);
    }
  }

  // renderer
  cuber::DxCubeRenderer renderer(platform.GetDevice());

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
      renderer.Render(app.projection, app.view, bvhPanel.GetCubes());
      platform.EndFrame(data);
    }
  }

  return 0;
}
