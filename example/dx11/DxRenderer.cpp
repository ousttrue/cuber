#include "DxRenderer.h"
#include "BvhSolver.h"
#include <DxCubeRenderer.h>
struct DxRendererImpl {
  BvhSolver bvhSolver_;
  cuber::DxCubeRenderer renderer_;
};

DxRenderer::DxRenderer() : impl_(new DxRendererImpl) {}
DxRenderer::~DxRenderer() { delete impl_; }
void DxRenderer::SetBvh(const std::shared_ptr<Bvh> &bvh) {
  impl_->bvhSolver_.Initialize(bvh);
}
void DxRenderer::SyncFrame(const BvhFrame &frame) {
  auto instances = impl_->bvhSolver_.ResolveFrame(frame);
  {
    std::lock_guard<std::mutex> lock(mutex_);
    instancies_.assign(instances.begin(), instances.end());
  }
}
void DxRenderer::RenderScene(RenderTime time, const float projection[16],
                             const float view[16]) {
  std::lock_guard<std::mutex> lock(mutex_);
  impl_->renderer_.Render<DirectX::XMFLOAT4X4>(projection, view, instancies_);
}
