#include <cuber/dx/DxCubeStereoRenderer.h>
#include <cuber/dx/shader.h>
#include <cuber/mesh.h>
#include <d3d11.h>
#include <d3dcompiler.h>

static auto SHADER = R"(
#pragma pack_matrix(row_major)

cbuffer ViewProjectionConstantBuffer : register(b0) {
    float4x4 ViewProjection[2]; // VPAndRTArrayIndexFromAnyShaderFeedingRasterizer
};
struct VSInput {
    float3 Pos : POSITION;
    float2 barycentric: BARYCENTRIC;
    float4 Row0: ROW0;
    float4 Row1: ROW1;
    float4 Row2: ROW2;
    float4 Row3: ROW3;
    float4 Color: COLOR0;
    uint instId : SV_InstanceID;
};
struct VSOutput {
    float4 Pos : SV_POSITION;
    float4 color: COLOR0;
    float2 barycentric: TEXCOORD;
    uint viewId : SV_RenderTargetArrayIndex;
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

VSOutput vs_main(VSInput IN) {
    VSOutput OUT;
    OUT.Pos = mul(mul(float4(IN.Pos, 1), transform(IN.Row0, IN.Row1, IN.Row2, IN.Row3)), ViewProjection[IN.instId % 2]);
    OUT.color = IN.Color;
    OUT.barycentric = IN.barycentric;
    OUT.viewId = IN.instId % 2;
    return OUT;
}

float grid (float2 vBC, float width) {
  float3 bary = float3(vBC.x, vBC.y, 1.0 - vBC.x - vBC.y);
  float3 d = fwidth(bary);
  float3 a3 = smoothstep(d * (width - 0.5), d * (width + 0.5), bary);
  return min(a3.x, a3.y);
}

float4 ps_main(VSOutput IN) : SV_TARGET {
  float value = grid(IN.barycentric, 1.0);
  return IN.color * float4(value, value, value, 1.0);
}
)";

namespace cuber::dx11 {
struct DxCubeStereoRendererImpl
{
  winrt::com_ptr<ID3D11Device> device_;
  winrt::com_ptr<ID3D11VertexShader> vertex_shader_;
  winrt::com_ptr<ID3D11PixelShader> pixel_shader_;
  winrt::com_ptr<ID3D11InputLayout> input_layout_;

  winrt::com_ptr<ID3D11Buffer> vertex_buffer_;
  winrt::com_ptr<ID3D11Buffer> index_buffer_;
  winrt::com_ptr<ID3D11Buffer> instance_buffer_;
  winrt::com_ptr<ID3D11Buffer> constant_buffer_;

  DxCubeStereoRendererImpl(const winrt::com_ptr<ID3D11Device>& device)
    : device_(device)
  {
    auto vs = CompileShader(SHADER, "vs_main", "vs_5_0");
    auto hr = device_->CreateVertexShader(
      vs->GetBufferPointer(), vs->GetBufferSize(), NULL, vertex_shader_.put());
    assert(SUCCEEDED(hr));

    auto ps = CompileShader(SHADER, "ps_main", "ps_5_0");
    hr = device_->CreatePixelShader(
      ps->GetBufferPointer(), ps->GetBufferSize(), NULL, pixel_shader_.put());
    assert(SUCCEEDED(hr));

    auto [vertices, indices, layouts] = Cube(false, true);

    uint32_t slots[] = {
      0, 0, 1, 1, 1, 1,1,
    };
    std::vector<D3D11_INPUT_ELEMENT_DESC> inputElementDesc;
    for (uint32_t i = 0; i < layouts.size(); ++i) {
      auto& layout = layouts[i];
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

    hr = device_->CreateInputLayout(inputElementDesc.data(),
                                    inputElementDesc.size(),
                                    vs->GetBufferPointer(),
                                    vs->GetBufferSize(),
                                    input_layout_.put());
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
      hr = device_->CreateBuffer(
        &vertex_buff_desc, &sr_data, vertex_buffer_.put());
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
      hr =
        device_->CreateBuffer(&index_buff_desc, &sr_data, index_buffer_.put());
      assert(SUCCEEDED(hr));
    }

    {
      D3D11_BUFFER_DESC instance_buff_desc = {
        .ByteWidth = static_cast<uint32_t>(sizeof(Instance) * 65535),
        .Usage = D3D11_USAGE_DEFAULT,
        .BindFlags = D3D11_BIND_VERTEX_BUFFER,
      };
      hr = device_->CreateBuffer(
        &instance_buff_desc, nullptr, instance_buffer_.put());
      assert(SUCCEEDED(hr));
    }

    {
      D3D11_BUFFER_DESC desc = {
        // 2
        .ByteWidth = sizeof(DirectX::XMFLOAT4X4) * 2,
        .Usage = D3D11_USAGE_DEFAULT,
        .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
      };
      HRESULT hr =
        device_->CreateBuffer(&desc, nullptr, constant_buffer_.put());
      assert(SUCCEEDED(hr));
    };
  }

  void Render(const float projection[16],
              const float view[16],
              const float rightProjection[16],
              const float rightView[16],
              const Instance* data,
              uint32_t instanceCount)
  {
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
      .right = static_cast<uint32_t>(sizeof(Instance) * instanceCount),
      .bottom = 1,
      .back = 1,
    };
    context->UpdateSubresource(
      instance_buffer_.get(), 0, &box, data, sizeof(Instance), 0);

    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->IASetInputLayout(input_layout_.get());
    ID3D11Buffer* vb[] = {
      vertex_buffer_.get(),
      instance_buffer_.get(),
    };
    uint32_t strides[] = {
      sizeof(grapho::Vertex),
      sizeof(Instance),
    };
    uint32_t offsets[] = { 0, 0 };
    context->IASetVertexBuffers(0, std::size(vb), vb, strides, offsets);
    context->IASetIndexBuffer(index_buffer_.get(), DXGI_FORMAT_R32_UINT, 0);

    // 2
    DirectX::XMFLOAT4X4 vp[2];
    {
      // left
      auto v = DirectX::XMLoadFloat4x4((const DirectX::XMFLOAT4X4*)view);
      auto p = DirectX::XMLoadFloat4x4((const DirectX::XMFLOAT4X4*)projection);
      DirectX::XMStoreFloat4x4(&vp[0], v * p);
    }
    {
      // right
      auto v = DirectX::XMLoadFloat4x4((const DirectX::XMFLOAT4X4*)rightView);
      auto p =
        DirectX::XMLoadFloat4x4((const DirectX::XMFLOAT4X4*)rightProjection);
      DirectX::XMStoreFloat4x4(&vp[1], v * p);
    }
    context->UpdateSubresource(constant_buffer_.get(), 0, NULL, vp, 0, 0);

    ID3D11Buffer* cb[]{ constant_buffer_.get() };
    context->VSSetConstantBuffers(0, 1, cb);
    // 2
    context->DrawIndexedInstanced(CUBE_INDEX_COUNT, instanceCount * 2, 0, 0, 0);
  }
};

DxCubeStereoRenderer::DxCubeStereoRenderer(
  const winrt::com_ptr<ID3D11Device>& device)
  : impl_(new DxCubeStereoRendererImpl(device))
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
