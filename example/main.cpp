#include "GlRenderer.h"
#include "GuiApp.h"
#include "GuiWindow.h"
#include <Windows.h>

#include <GL/GL.h>

int main(int, char **) {
  GuiWindow gui;
  auto window = gui.Create();
  if (!window) {
    return 1;
  }

  GuiApp app(window, gui.GlslVersion());

  GlRenderer renderer;
  CameraMatrix camera = {};

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
    renderer.RenderScene(*time, camera);

    // app
    app.RenderGui();

    gui.EndFrame();
  }

  return 0;
}
