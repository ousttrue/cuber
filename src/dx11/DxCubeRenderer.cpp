#include "DxCubeRenderer.h"
#include <d3d11.h>
#include <d3dcompiler.h>

auto SHADER = R"(
#pragma pack_matrix(row_major)

// 定義
cbuffer c0
{
    float4x4 ProjectionMatrix;
    float4x4 ViewMatrix;
};

/* vertex attributes go here to input to the vertex shader */
struct vs_in {
    float3 position_local : POS;
};

/* outputs from vertex shader go here. can be interpolated to pixel shader */
struct vs_out {
    float4 position_clip : SV_POSITION; // required output of VS
};

vs_out vs_main(vs_in input) {
  vs_out output = (vs_out)0; // zero the memory first
  output.position_clip = mul(float4(input.position_local, 1.0), mul(ViewMatrix, ProjectionMatrix));
  return output;
}

float4 ps_main(vs_out input) : SV_TARGET {
  return float4( 1.0, 0.0, 1.0, 1.0 ); // must return an RGBA colour
}
)";

// entry: "vs_main"
// target: "vs_5_0"
static winrt::com_ptr<ID3DBlob>
CompileShader(std::string_view src, const char *entry, const char *target) {
  UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
  flags |= D3DCOMPILE_DEBUG; // add more debug output
#endif
  winrt::com_ptr<ID3DBlob> vs_blob_ptr;
  winrt::com_ptr<ID3DBlob> error_blob;
  auto hr = D3DCompile(src.data(), src.size(), "shaders.hlsl", nullptr,
                       D3D_COMPILE_STANDARD_FILE_INCLUDE, entry, target, flags,
                       0, vs_blob_ptr.put(), error_blob.put());
  if (FAILED(hr)) {
    if (error_blob) {
      OutputDebugStringA((char *)error_blob->GetBufferPointer());
    }
    return {};
  }
  return vs_blob_ptr;
}

const float size = 5.0f;
float vertex_data_array[] = {
    0.0f,  size,  0.0f, // point at top
    size,  -size, 0.0f, // point at bottom-right
    -size, -size, 0.0f, // point at bottom-left
};
UINT vertex_stride = 3 * sizeof(float);
UINT vertex_offset = 0;
UINT vertex_count = 3;

struct Constant {
  DirectX::XMFLOAT4X4 projection;
  DirectX::XMFLOAT4X4 view;
};

namespace cuber {
struct DxCubeRendererImpl {
  winrt::com_ptr<ID3D11Device> device_;
  winrt::com_ptr<ID3D11VertexShader> vertex_shader_;
  winrt::com_ptr<ID3D11PixelShader> pixel_shader_;
  winrt::com_ptr<ID3D11InputLayout> input_layout_;
  winrt::com_ptr<ID3D11Buffer> vertex_buffer_;

  winrt::com_ptr<ID3D11Buffer> constant_buffer_;

  DxCubeRendererImpl(const winrt::com_ptr<ID3D11Device> &device)
      : device_(device) {
    auto vs = CompileShader(SHADER, "vs_main", "vs_5_0");
    auto hr =
        device_->CreateVertexShader(vs->GetBufferPointer(), vs->GetBufferSize(),
                                    NULL, vertex_shader_.put());
    assert(SUCCEEDED(hr));

    auto ps = CompileShader(SHADER, "ps_main", "ps_5_0");
    hr = device_->CreatePixelShader(ps->GetBufferPointer(), ps->GetBufferSize(),
                                    NULL, pixel_shader_.put());
    assert(SUCCEEDED(hr));

    D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
        {"POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        /*
        { "COL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
        D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }, { "NOR",
        0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,
        D3D11_INPUT_PER_VERTEX_DATA, 0 }, { "TEX", 0, DXGI_FORMAT_R32G32_FLOAT,
        0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        */
    };
    hr = device_->CreateInputLayout(
        inputElementDesc, ARRAYSIZE(inputElementDesc), vs->GetBufferPointer(),
        vs->GetBufferSize(), input_layout_.put());
    assert(SUCCEEDED(hr));

    { /*** load mesh data into vertex buffer **/
      D3D11_BUFFER_DESC vertex_buff_descr = {
          .ByteWidth = sizeof(vertex_data_array),
          .Usage = D3D11_USAGE_DEFAULT,
          .BindFlags = D3D11_BIND_VERTEX_BUFFER,
      };
      D3D11_SUBRESOURCE_DATA sr_data = {0};
      sr_data.pSysMem = vertex_data_array;
      hr = device_->CreateBuffer(&vertex_buff_descr, &sr_data,
                                 vertex_buffer_.put());
      assert(SUCCEEDED(hr));
    }

    {
      D3D11_BUFFER_DESC desc = {
          .ByteWidth = sizeof(Constant),
          .Usage = D3D11_USAGE_DEFAULT,
          .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
      };
      HRESULT hr =
          device_->CreateBuffer(&desc, nullptr, constant_buffer_.put());
      assert(SUCCEEDED(hr));
    };
  }

  void Render(const float projection[16], const float view[16],
              const void *data, uint32_t instanceCount) {
    winrt::com_ptr<ID3D11DeviceContext> context;
    device_->GetImmediateContext(context.put());

    context->VSSetShader(vertex_shader_.get(), NULL, 0);
    context->PSSetShader(pixel_shader_.get(), NULL, 0);

    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->IASetInputLayout(input_layout_.get());
    ID3D11Buffer *vb[] = {vertex_buffer_.get()};
    context->IASetVertexBuffers(0, 1, vb, &vertex_stride, &vertex_offset);

    Constant constant_value{
        *((const DirectX::XMFLOAT4X4 *)projection),
        *((const DirectX::XMFLOAT4X4 *)view),
    };
    context->UpdateSubresource(constant_buffer_.get(), 0, NULL, &constant_value,
                               0, 0);

    ID3D11Buffer *cb[]{constant_buffer_.get()};
    context->VSSetConstantBuffers(0, 1, cb);
    context->Draw(vertex_count, 0);
  }
};

DxCubeRenderer::DxCubeRenderer(const winrt::com_ptr<ID3D11Device> &device)
    : impl_(new DxCubeRendererImpl(device)) {}
DxCubeRenderer::~DxCubeRenderer() { delete impl_; }
void DxCubeRenderer::Render(const float projection[16], const float view[16],
                            const void *data, uint32_t instanceCount) {
  impl_->Render(projection, view, data, instanceCount);
}

} // namespace cuber