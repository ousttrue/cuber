#include "GlRenderer.h"
#include "BvhSolver.h"
#include <GL/glew.h>
#include <iostream>

struct GlRendererImpl {
  BvhSolver bvhSolver_;

  GlRendererImpl() {
    std::cout << "GL_VERSION: " << glGetString(GL_VERSION) << std::endl;
    glewInit();
    std::cout << "GLEW_VERSION: " << glewGetString(GLEW_VERSION) << std::endl;
  }
  ~GlRendererImpl() {}
};

GlRenderer::GlRenderer() : impl_(new GlRendererImpl) {}

GlRenderer::~GlRenderer() { delete impl_; }

void GlRenderer::SetBvh(const std::shared_ptr<struct Bvh> &bvh) {
  impl_->bvhSolver_.Initialize(bvh);
}

void GlRenderer::SetFrame(const BvhFrame &frame) {
  auto instances = impl_->bvhSolver_.ResolveFrame(frame);
  {
    std::lock_guard<std::mutex> lock(mutex_);
    instancies_.assign(instances.begin(), instances.end());
  }
}

void GlRenderer::RenderScene(RenderTime time, const float projection[16],
                             const float view[16]) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (!instancies_.empty()) {
    cubes.Render<DirectX::XMFLOAT4X4>(projection, view, instancies_);
  }
}