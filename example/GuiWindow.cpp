#include "GuiWindow.h"
#include <stdexcept>
#include <stdio.h>

const char *glsl_version = nullptr;

#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

static void glfw_error_callback(int error, const char *description) {
  fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

GuiWindow::GuiWindow() {
  // Setup window
  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit()) {
    throw std::runtime_error("glfwInit");
  }
}

GuiWindow::~GuiWindow() {
  glfwDestroyWindow(window_);
  glfwTerminate();
}

GLFWwindow *GuiWindow::Create() {

  // GL 3.2 + GLSL 150
  glsl_version = "#version 150";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);           // Required on Mac

  // Create window with graphics context
  window_ = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", NULL,
                             NULL);
  if (!window_) {
    return nullptr;
  }
  glfwMakeContextCurrent(window_);
  glfwSwapInterval(1); // Enable vsync
  return window_;
}

std::optional<GlfwTime> GuiWindow::NewFrame(int *display_w, int *display_h) {
  if (glfwWindowShouldClose(window_)) {
    return {};
  }
  // Poll and handle events (inputs, window resize, etc.)
  // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to
  // tell if dear imgui wants to use your inputs.
  // - When io.WantCaptureMouse is true, do not dispatch mouse input data to
  // your main application, or clear/overwrite your copy of the mouse data.
  // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input
  // data to your main application, or clear/overwrite your copy of the
  // keyboard data. Generally you may always pass all inputs to dear imgui,
  // and hide them from your application based on those two flags.
  glfwPollEvents();

  glfwGetFramebufferSize(window_, display_w, display_h);
  return GlfwTime(glfwGetTime());
}

void GuiWindow::EndFrame() { glfwSwapBuffers(window_); }

const char *GuiWindow::GlslVersion() const { return glsl_version; }
