#include "BvhPanel.h"
#include "GlfwPlatform.h"
#include "GuiApp.h"
#include <cuber/gl3/GlCubeRenderer.h>

int main(int argc, char **argv) {
  // imgui
  GuiApp app(false);

  // window
  GlfwPlatform platform;
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

  // cuber
  cuber::gl3::GlCubeRenderer renderer;

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
