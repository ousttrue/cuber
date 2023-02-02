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

  // Main loop
  int display_w, display_h;
  while (gui.NewFrame(&display_w, &display_h)) {

    app.UpdateGui();

    // scene
    glViewport(0, 0, display_w, display_h);
    glClearColor(app.clear_color[0], app.clear_color[1], app.clear_color[2],
                 app.clear_color[3]);
    glClear(GL_COLOR_BUFFER_BIT);

    // app
    app.RenderGui();

    gui.EndFrame();
  }

  return 0;
}
