#include "BvhPanel.h"
#include "DxPlatform.h"
#include "GuiApp.h"
#include <cuber/dx/DxCubeRenderer.h>
#include <cuber/dx/DxLineRenderer.h>

// Main code
int main(int argc, char **argv) {
  // imgui
  GuiApp app;

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
  cuber::dx11::DxCubeRenderer cubeRenderer(platform.GetDevice());
  cuber::dx11::DxLineRenderer lineRenderer(platform.GetDevice());

  std::vector<grapho::LineVertex> lines;
  cuber::PushGrid(lines);

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
      cubeRenderer.Render(app.projection, app.view, bvhPanel.GetCubes(),
                          bvhPanel.GetCubeAttributes());
      lineRenderer.Render(app.projection, app.view, lines);
      platform.EndFrame(data);
    }
  }

  return 0;
}
