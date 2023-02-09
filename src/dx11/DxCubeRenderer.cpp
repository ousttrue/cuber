#include "DxCubeRenderer.h"
#include <d3d11.h>

namespace cuber {
struct DxCubeRendererImpl {
  winrt::com_ptr<ID3D11Device> device_;

  DxCubeRendererImpl(const winrt::com_ptr<ID3D11Device> &device)
      : device_(device) {}
};

DxCubeRenderer::DxCubeRenderer(const winrt::com_ptr<ID3D11Device> &device)
    : impl_(new DxCubeRendererImpl(device)) {}
DxCubeRenderer::~DxCubeRenderer() { delete impl_; }
void DxCubeRenderer::Render(const float projection[16], const float view[16],
                            const void *data, uint32_t instanceCount) {}

} // namespace cuber