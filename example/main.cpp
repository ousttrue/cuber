#include "Animation.h"
#include "GlRenderer.h"
#include "GuiApp.h"
#include "GuiWindow.h"
#include "TurnTable.h"
#include <DirectXMath.h>
#include <asio.hpp>
#include <iostream>
#include <thread>

// must after asio.hpp
#include <Windows.h>

// must after Windows.h
#include <GL/GL.h>

int main(int argc, char **argv) {
  asio::io_context io;

  GuiWindow gui;
  auto window = gui.Create();
  if (!window) {
    return 1;
  }

  GuiApp app(window, gui.GlslVersion());

  GlRenderer renderer;

  Animation animation(io);
  animation.OnFrame(
      [&renderer](auto instances) { renderer.SetInstances(instances); });

  if (argc > 1) {
    animation.Load(argv[1]);
  }

  // start
  auto work = asio::make_work_guard(io);
  std::thread thread([&io]() {
    try {
      io.run();
      std::cout << "[asio] end" << std::endl;
    } catch (std::exception const &e) {
      std::cout << "[asio] catch" << e.what() << std::endl;
    }
  });

  // main loop
  int display_w, display_h;
  while (auto time = gui.NewFrame(&display_w, &display_h)) {
    app.UpdateGui();

    // render
    glViewport(0, 0, display_w, display_h);
    glClearColor(app.clear_color[0], app.clear_color[1], app.clear_color[2],
                 app.clear_color[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // scene
    renderer.RenderScene(*time, app.projection, app.view);

    // app
    app.RenderGui();

    gui.EndFrame();
  }

  animation.Stop();
  work.reset();
  thread.join();

  return 0;
}
