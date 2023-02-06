#include "Animation.h"
#include "Bvh.h"
#include "GlRenderer.h"
#include "GuiApp.h"
#include "GuiWindow.h"
#include "TurnTable.h"
#include "UdpSender.h"
#include <imgui.h>
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
  animation.OnFrame([&renderer](auto time, auto instances) {
    renderer.SetInstances(instances);
  });

  asio::ip::udp::endpoint ep(asio::ip::address::from_string("127.0.0.1"),
                             54345);
  UdpSender sender(io);
  animation.OnFrame([&sender, &ep](auto time, auto instances) {
    sender.SendFrame(ep, time, instances);
  });

  std::shared_ptr<Bvh> bvh;
  if (argc > 1) {
    if ((bvh = animation.Load(argv[1]))) {
      sender.SendSkeleton(ep, bvh);
    }
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
    {
      // imgui
      app.UpdateGui();

      if (bvh) {
        // bvh panel
        ImGui::Begin("BVH");

        ImGui::LabelText("bvh", "%zu joints", bvh->joints.size());

        if (ImGui::Button("send skeleton")) {
          sender.SendSkeleton(ep, bvh);
        }

        ImGui::End();
      }
    }

    {
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
  }

  animation.Stop();
  work.reset();
  thread.join();

  return 0;
}
