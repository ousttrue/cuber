#include "GlRenderer.h"
#include "GuiApp.h"
#include "GuiWindow.h"
#include "TurnTable.h"
#include <DirectXMath.h>
#include <Windows.h>

#include <GL/GL.h>

int main(int argc, char **argv) {
  GuiWindow gui;
  auto window = gui.Create();
  if (!window) {
    return 1;
  }

  GuiApp app(window, gui.GlslVersion());

  GlRenderer renderer;
  if (argc > 1) {
    renderer.Load(argv[1]);
  }

  // main loop
  int display_w, display_h;
  while (auto time = gui.NewFrame(&display_w, &display_h)) {
    app.UpdateGui();

    // render
    glViewport(0, 0, display_w, display_h);
    glClearColor(app.clear_color[0], app.clear_color[1], app.clear_color[2],
                 app.clear_color[3]);
    glClear(GL_COLOR_BUFFER_BIT);

    // scene
    renderer.RenderScene(*time, app.projection, app.view);

    // app
    app.RenderGui();

    gui.EndFrame();
  }

  return 0;
}
