#include "GlRenderer.h"
#include <GL/glew.h>
#include <iostream>

struct GlRendererImpl {
public:
  GlRendererImpl() {
    std::cout << "GL_VERSION: " << glGetString(GL_VERSION) << std::endl;
    glewInit();
    std::cout << "GLEW_VERSION: " << glewGetString(GLEW_VERSION) << std::endl;
  }
  ~GlRendererImpl() {}
};

GlRenderer::GlRenderer() : impl_(new GlRendererImpl) {}

GlRenderer::~GlRenderer() { delete impl_; }

void GlRenderer::SetInstances(std::span<DirectX::XMFLOAT4X4> instances) {
  std::lock_guard<std::mutex> lock(mutex_);
  insancies_.assign(instances.begin(), instances.end());
}

void GlRenderer::RenderScene(RenderTime time, const float projection[16],
                             const float view[16]) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (!insancies_.empty()) {
    auto begin = (cuber::Instance *)&*insancies_.begin();
    auto end = begin + insancies_.size();
    cubes.Render(projection, view, {begin, end});
  }
}
