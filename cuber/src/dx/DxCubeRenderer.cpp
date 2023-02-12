#include <cuber/dx/DxCubeRenderer.h>
#include <cuber/dx/shader.h>
#include <cuber/mesh.h>
#include <d3d11.h>
#include <d3dcompiler.h>

static auto SHADER = R"(
#pragma pack_matrix(row_major)
float4x4 VP;
struct vs_in {
    float3 pos: POSITION;
    float2 barycentric: BARYCENTRIC;
    float4 row0: ROW0;
    float4 row1: ROW1;
    float4 row2: ROW2;
    float4 row3: ROW3;    
    uint instanceID : SV_InstanceID;
 };
struct vs_out {
    float4 position_clip: SV_POSITION;
    float2 barycentric: TEXCOORD;
};

float4x4 transform(float4 r0, float4 r1, float4 r2, float4 r3)
{
  return float4x4(
    r0.x, r0.y, r0.z, r0.w,
    r1.x, r1.y, r1.z, r1.w,
    r2.x, r2.y, r2.z, r2.w,
    r3.x, r3.y, r3.z, r3.w
  );
}

vs_out vs_main(vs_in IN) {
  vs_out OUT = (vs_out)0; // zero the memory first
  OUT.position_clip = mul(float4(IN.pos, 1.0), mul(transform(IN.row0, IN.row1, IN.row2, IN.row3), VP));
  OUT.barycentric = IN.barycentric;
  return OUT;
}

float grid (float2 vBC, float width) {
  float3 bary = float3(vBC.x, vBC.y, 1.0 - vBC.x - vBC.y);
  float3 d = fwidth(bary);
  float3 a3 = smoothstep(d * (width - 0.5), d * (width + 0.5), bary);
  return min(a3.x, a3.y);
}

float4 ps_main(vs_out IN) : SV_TARGET {
  float value = grid(IN.barycentric, 1.0);
  return float4(value, value, value, 1.0);
}
)";

namespace cuber {
struct DxCubeRendererImpl {
  winrt::com_ptr<ID3D11Device> device_;
  winrt::com_ptr<ID3D11VertexShader> vertex_shader_;
  winrt::com_ptr<ID3D11PixelShader> pixel_shader_;
  winrt::com_ptr<ID3D11InputLayout> input_layout_;

  winrt::com_ptr<ID3D11Buffer> vertex_buffer_;
  winrt::com_ptr<ID3D11Buffer> index_buffer_;
  winrt::com_ptr<ID3D11Buffer> instance_buffer_;
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

    auto [vertices, indices, layouts] = cuber::Cube(false, false);

    uint32_t slots[] = {
        0, 0, 1, 1, 1, 1,
    };
    std::vector<D3D11_INPUT_ELEMENT_DESC> inputElementDesc;
    for (uint32_t i = 0; i < layouts.size(); ++i) {
      auto &layout = layouts[i];
      inputElementDesc.push_back(D3D11_INPUT_ELEMENT_DESC{
          .SemanticName = layout.id.semantic_name.c_str(),
          .SemanticIndex = layout.id.semantic_index,
          .Format = DxgiFormat(layout),
          .InputSlot = slots[i],
          .AlignedByteOffset = layout.offset,
          .InputSlotClass = layout.divisor ? D3D11_INPUT_PER_INSTANCE_DATA
                                           : D3D11_INPUT_PER_VERTEX_DATA,
          .InstanceDataStepRate = layout.divisor,
      });
    }

    hr = device_->CreateInputLayout(
        inputElementDesc.data(), inputElementDesc.size(),
        vs->GetBufferPointer(), vs->GetBufferSize(), input_layout_.put());
    assert(SUCCEEDED(hr));

    {
      D3D11_BUFFER_DESC vertex_buff_desc = {
          .ByteWidth =
              static_cast<uint32_t>(sizeof(vertices[0]) * vertices.size()),
          .Usage = D3D11_USAGE_DEFAULT,
          .BindFlags = D3D11_BIND_VERTEX_BUFFER,
      };
      D3D11_SUBRESOURCE_DATA sr_data = {
          .pSysMem = vertices.data(),
      };
      hr = device_->CreateBuffer(&vertex_buff_desc, &sr_data,
                                 vertex_buffer_.put());
      assert(SUCCEEDED(hr));
    }

    {
      D3D11_BUFFER_DESC index_buff_desc = {
          .ByteWidth = static_cast<uint32_t>(sizeof(uint32_t) * indices.size()),
          .Usage = D3D11_USAGE_DEFAULT,
          .BindFlags = D3D11_BIND_INDEX_BUFFER,
      };
      D3D11_SUBRESOURCE_DATA sr_data = {
          .pSysMem = indices.data(),
      };
      hr = device_->CreateBuffer(&index_buff_desc, &sr_data,
                                 index_buffer_.put());
      assert(SUCCEEDED(hr));
    }

    {
      D3D11_BUFFER_DESC instance_buff_desc = {
          .ByteWidth =
              static_cast<uint32_t>(sizeof(DirectX::XMFLOAT4X4) * 65535),
          .Usage = D3D11_USAGE_DEFAULT,
          .BindFlags = D3D11_BIND_VERTEX_BUFFER,
      };
      hr = device_->CreateBuffer(&instance_buff_desc, nullptr,
                                 instance_buffer_.put());
      assert(SUCCEEDED(hr));
    }

    {
      D3D11_BUFFER_DESC desc = {
          .ByteWidth = sizeof(DirectX::XMFLOAT4X4),
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
    if (instanceCount == 0) {
      return;
    }
    winrt::com_ptr<ID3D11DeviceContext> context;
    device_->GetImmediateContext(context.put());

    context->VSSetShader(vertex_shader_.get(), NULL, 0);
    context->PSSetShader(pixel_shader_.get(), NULL, 0);

    D3D11_BOX box{
        .left = 0,
        .top = 0,
        .front = 0,
        .right =
            static_cast<uint32_t>(sizeof(DirectX::XMFLOAT4X4) * instanceCount),
        .bottom = 1,
        .back = 1,
    };
    context->UpdateSubresource(instance_buffer_.get(), 0, &box, data,
                               sizeof(DirectX::XMFLOAT4X4), 0);

    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->IASetInputLayout(input_layout_.get());
    ID3D11Buffer *vb[] = {
        vertex_buffer_.get(),
        instance_buffer_.get(),
    };
    uint32_t strides[] = {
        sizeof(Vertex),
        sizeof(DirectX::XMFLOAT4X4),
    };
    uint32_t offsets[] = {0, 0};
    context->IASetVertexBuffers(0, std::size(vb), vb, strides, offsets);
    context->IASetIndexBuffer(index_buffer_.get(), DXGI_FORMAT_R32_UINT, 0);

    auto v = DirectX::XMLoadFloat4x4((const DirectX::XMFLOAT4X4 *)view);
    auto p = DirectX::XMLoadFloat4x4((const DirectX::XMFLOAT4X4 *)projection);
    DirectX::XMFLOAT4X4 vp;
    DirectX::XMStoreFloat4x4(&vp, v * p);
    context->UpdateSubresource(constant_buffer_.get(), 0, NULL, &vp, 0, 0);

    ID3D11Buffer *cb[]{constant_buffer_.get()};
    context->VSSetConstantBuffers(0, 1, cb);
    context->DrawIndexedInstanced(CUBE_INDEX_COUNT, instanceCount, 0, 0, 0);
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