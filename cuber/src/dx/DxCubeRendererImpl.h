#pragma once
#include <cuber/mesh.h>
#include <d3d11.h>
#include <winrt/base.h>

namespace grapho {
namespace dx11 {
struct Drawable;
}
}

namespace cuber {
namespace dx11 {

struct DxCubeRendererImpl
{
  winrt::com_ptr<ID3D11Device> device_;
  bool stereo_;
  winrt::com_ptr<ID3D11VertexShader> vertex_shader_;
  winrt::com_ptr<ID3D11PixelShader> pixel_shader_;
  winrt::com_ptr<ID3D11Buffer> instance_buffer_;
  std::shared_ptr<grapho::dx11::Drawable> drawable_;
  winrt::com_ptr<ID3D11Buffer> constant_buffer_;

  Pallete pallete_;
  winrt::com_ptr<ID3D11Buffer> pallete_buffer_;

  DxCubeRendererImpl(const winrt::com_ptr<ID3D11Device>& device, bool stereo);
  void Render(const float projection[16],
              const float view[16],
              const void* data,
              uint32_t instanceCount);
  void Render(const float projection[16],
              const float view[16],
              const float rightProjection[16],
              const float rightView[16],
              const Instance* data,
              uint32_t instanceCount);
};

}
}
