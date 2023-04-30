#include "DxCubeRendererImpl.h"
#include <cuber/dx/DxCubeStereoRenderer.h>

namespace cuber::dx11 {
DxCubeStereoRenderer::DxCubeStereoRenderer(
  const winrt::com_ptr<ID3D11Device>& device)
  : impl_(new DxCubeRendererImpl(device, true))
{
}
DxCubeStereoRenderer::~DxCubeStereoRenderer()
{
  delete impl_;
}
void
DxCubeStereoRenderer::Render(const float projection[16],
                             const float view[16],
                             const float rightProjection[16],
                             const float rightView[16],
                             const Instance* data,
                             uint32_t instanceCount)
{
  impl_->Render(
    projection, view, rightProjection, rightView, data, instanceCount);
}

} // namespace cuber::dx11
