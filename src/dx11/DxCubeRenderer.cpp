#include "DxCubeRenderer.h"

namespace cuber {
struct DxCubeRendererImpl {};

DxCubeRenderer::DxCubeRenderer() : impl_(new DxCubeRendererImpl) {}
DxCubeRenderer::~DxCubeRenderer() { delete impl_; }
void DxCubeRenderer::Render(const float projection[16], const float view[16],
                            const void *data, uint32_t instanceCount) {}

} // namespace cuber