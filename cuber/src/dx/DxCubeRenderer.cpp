#include <cuber/dx/DxCubeRenderer.h>
#include <cuber/mesh.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <grapho/dx11/buffer.h>
#include <grapho/dx11/drawable.h>
#include <grapho/dx11/shader.h>

static auto SHADER = R"(
#pragma pack_matrix(row_major)
float4x4 VP;
struct vs_in {
    float4 pos: POSITION;
    float4 uv_barycentric: TEXCOORD;
    float4 row0: ROW0;
    float4 row1: ROW1;
    float4 row2: ROW2;
    float4 row3: ROW3;    
    float4 positiveXyzFlag: FACE0;
    float4 negativeXyzFlag: FACE1;
    uint instanceID : SV_InstanceID;
};
struct vs_out {
    float4 position_clip: SV_POSITION;
    float4 uv_barycentric: TEXCOORD;
    uint3 paletteFlagFlag: COLOR;
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
  OUT.position_clip = mul(float4(IN.pos.xyz, 1), mul(transform(IN.row0, IN.row1, IN.row2, IN.row3), VP));
  OUT.uv_barycentric = IN.uv_barycentric;
  if(IN.pos.w==0.0)
  {
    OUT.paletteFlagFlag = uint3(IN.positiveXyzFlag.x, 
      IN.positiveXyzFlag.w,
      IN.negativeXyzFlag.w);
  }
  else if(IN.pos.w==1.0)
  {
    OUT.paletteFlagFlag = uint3(IN.positiveXyzFlag.y, 
      IN.positiveXyzFlag.w,
      IN.negativeXyzFlag.w);
  }
  else if(IN.pos.w==2.0)
  {
    OUT.paletteFlagFlag = uint3(IN.positiveXyzFlag.z, 
      IN.positiveXyzFlag.w,
      IN.negativeXyzFlag.w);
  }
  else if(IN.pos.w==3.0)
  {
    OUT.paletteFlagFlag = uint3(IN.negativeXyzFlag.x, 
      IN.positiveXyzFlag.w,
      IN.negativeXyzFlag.w);
  }
  else if(IN.pos.w==4.0)
  {
    OUT.paletteFlagFlag = uint3(IN.negativeXyzFlag.y, 
      IN.positiveXyzFlag.w,
      IN.negativeXyzFlag.w);
  }
  else if(IN.pos.w==5.0)
  {
    OUT.paletteFlagFlag = uint3(IN.negativeXyzFlag.z, 
      IN.positiveXyzFlag.w,
      IN.negativeXyzFlag.w);
  }
  else{
    OUT.paletteFlagFlag = uint3(0, 
      IN.positiveXyzFlag.w,
      IN.negativeXyzFlag.w);
  }

  return OUT;
}

cbuffer c0
{
  float4 colors[64];
  float4 textures[64];
};
Texture2D texture1;
SamplerState sampler1;
Texture2D texture2;
SamplerState sampler2;
Texture2D texture3;
SamplerState sampler3;

float grid (float2 vBC, float width) {
  float3 bary = float3(vBC.x, vBC.y, 1.0 - vBC.x - vBC.y);
  float3 d = fwidth(bary);
  float3 a3 = smoothstep(d * (width - 0.5), d * (width + 0.5), bary);
  return min(a3.x, a3.y);
}

float4 ps_main(vs_out IN) : SV_TARGET {
  float value = grid(IN.uv_barycentric.zw, 1.0);
  float4 border = float4(value, value, value, 1.0);
  float4 color = colors[IN.paletteFlagFlag.x];
  float4 texel;
  if(textures[IN.paletteFlagFlag.x].x==1.0)
  {
    texel = texture1.Sample(sampler1, IN.uv_barycentric.xy);
  }
  else if(textures[IN.paletteFlagFlag.x].x==2.0)
  {
    texel = texture2.Sample(sampler2, IN.uv_barycentric.xy);
  }
  else if(textures[IN.paletteFlagFlag.x].x==3.0)
  {
    texel = texture3.Sample(sampler3, IN.uv_barycentric.xy);
  }
  else{
    texel = float4(1, 1, 1, 1);
  }

  return texel * color * border;
}
)";

namespace cuber::dx11 {
struct DxCubeRendererImpl
{
  winrt::com_ptr<ID3D11Device> device_;
  winrt::com_ptr<ID3D11VertexShader> vertex_shader_;
  winrt::com_ptr<ID3D11PixelShader> pixel_shader_;
  winrt::com_ptr<ID3D11Buffer> instance_buffer_;
  std::shared_ptr<grapho::dx11::Drawable> drawable_;
  winrt::com_ptr<ID3D11Buffer> constant_buffer_;

  Pallete pallete_;
  winrt::com_ptr<ID3D11Buffer> pallete_buffer_;

  DxCubeRendererImpl(const winrt::com_ptr<ID3D11Device>& device)
    : device_(device)
  {
    auto vs = grapho::dx11::CompileShader(SHADER, "vs_main", "vs_5_0");
    if (!vs) {
      OutputDebugStringA(vs.error().c_str());
    }
    auto hr = device_->CreateVertexShader((*vs)->GetBufferPointer(),
                                          (*vs)->GetBufferSize(),
                                          NULL,
                                          vertex_shader_.put());
    assert(SUCCEEDED(hr));

    auto ps = grapho::dx11::CompileShader(SHADER, "ps_main", "ps_5_0");
    if (!ps) {
      OutputDebugStringA(ps.error().c_str());
    }
    hr = device_->CreatePixelShader((*ps)->GetBufferPointer(),
                                    (*ps)->GetBufferSize(),
                                    NULL,
                                    pixel_shader_.put());
    assert(SUCCEEDED(hr));

    auto [vertices, indices, layouts] = cuber::Cube(false, false);

    auto vertex_buffer = grapho::dx11::CreateVertexBuffer(device_, vertices);
    instance_buffer_ = grapho::dx11::CreateVertexBuffer(
      device_, sizeof(Instance) * 65535, nullptr);
    auto index_buffer = grapho::dx11::CreateIndexBuffer(device_, indices);

    grapho::dx11::VertexSlot slots[]{
      {
        .VertexBuffer = vertex_buffer,
        .Stride = sizeof(Vertex),
      },
      {
        .VertexBuffer = instance_buffer_,
        .Stride = sizeof(Instance),
      },
    };

    drawable_ = grapho::dx11::Drawable::Create(
      device_, *vs, layouts, slots, index_buffer);
    assert(drawable_);

    constant_buffer_ = grapho::dx11::CreateConstantBuffer(
      device_, sizeof(DirectX::XMFLOAT4X4), nullptr);
    assert(constant_buffer_);

    pallete_buffer_ =
      grapho::dx11::CreateConstantBuffer(device_, sizeof(pallete_), &pallete_);
    assert(pallete_buffer_);
  }

  void Render(const float projection[16],
              const float view[16],
              const void* data,
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
      .right = static_cast<uint32_t>(sizeof(cuber::Instance) * instanceCount),
      .bottom = 1,
      .back = 1,
    };
    context->UpdateSubresource(
      instance_buffer_.get(), 0, &box, data, sizeof(Instance), 0);

    auto v = DirectX::XMLoadFloat4x4((const DirectX::XMFLOAT4X4*)view);
    auto p = DirectX::XMLoadFloat4x4((const DirectX::XMFLOAT4X4*)projection);
    DirectX::XMFLOAT4X4 vp;
    DirectX::XMStoreFloat4x4(&vp, v * p);
    context->UpdateSubresource(constant_buffer_.get(), 0, NULL, &vp, 0, 0);

    ID3D11Buffer* vscb[]{ constant_buffer_.get() };
    context->VSSetConstantBuffers(0, 1, vscb);
    ID3D11Buffer* pscb[]{ pallete_buffer_.get() };
    context->PSSetConstantBuffers(0, 1, pscb);

    drawable_->DrawInstance(context, CUBE_INDEX_COUNT, instanceCount);
  }
};

DxCubeRenderer::DxCubeRenderer(const winrt::com_ptr<ID3D11Device>& device)
  : m_impl(new DxCubeRendererImpl(device))
{
}
DxCubeRenderer::~DxCubeRenderer()
{
  delete m_impl;
}
void
DxCubeRenderer::Render(const float projection[16],
                       const float view[16],
                       const Instance* data,
                       uint32_t instanceCount)
{
  m_impl->Render(projection, view, data, instanceCount);
}

}
