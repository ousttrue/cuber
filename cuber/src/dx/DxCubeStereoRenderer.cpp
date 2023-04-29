#include <cuber/dx/DxCubeStereoRenderer.h>
#include <cuber/mesh.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <grapho/dx11/buffer.h>
#include <grapho/dx11/drawable.h>
#include <grapho/dx11/shader.h>

static auto SHADER = R"(
#pragma pack_matrix(row_major)

cbuffer ViewProjectionConstantBuffer : register(b0) {
    float4x4 ViewProjection[2]; // VPAndRTArrayIndexFromAnyShaderFeedingRasterizer
};
struct VSInput {
    float4 Pos : POSITION;
    float4 uv_barycentric: TEXCOORD;
    float4 Row0: ROW0;
    float4 Row1: ROW1;
    float4 Row2: ROW2;
    float4 Row3: ROW3;
    float4 positiveXyzFlag: FACE0;
    float4 negativeXyzFlag: FACE1;
    uint instId : SV_InstanceID;
};
struct VSOutput {
    float4 Pos : SV_POSITION;
    float4 uv_barycentric: TEXCOORD;
    uint3 paletteFlagFlag: COLOR;
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
  OUT.Pos = mul(mul(float4(IN.Pos.xyz, 1), transform(IN.Row0, IN.Row1, IN.Row2, IN.Row3)), ViewProjection[IN.instId % 2]);
  OUT.uv_barycentric = IN.uv_barycentric;
  OUT.viewId = IN.instId % 2;
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

  return OUT;
}

cbuffer c0
{
  float4 colors[64];
  float4 textures[64];
};
Texture2D texture0;
SamplerState sampler0;
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

float4 ps_main(VSOutput IN) : SV_TARGET {
  float value = grid(IN.uv_barycentric.zw, 1.0);
  float4 border = float4(value, value, value, 1.0);
  float4 color = colors[IN.paletteFlagFlag.x];
  float4 texel;
  if(textures[IN.paletteFlagFlag.x].x==0.0)
  {
    texel = texture0.Sample(sampler0, IN.uv_barycentric.xy);
  }
  else if(textures[IN.paletteFlagFlag.x].x==1.0)
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
struct DxCubeStereoRendererImpl
{
  winrt::com_ptr<ID3D11Device> device_;
  winrt::com_ptr<ID3D11VertexShader> vertex_shader_;
  winrt::com_ptr<ID3D11PixelShader> pixel_shader_;
  winrt::com_ptr<ID3D11Buffer> instance_buffer_;
  std::shared_ptr<grapho::dx11::Drawable> drawable_;
  winrt::com_ptr<ID3D11Buffer> constant_buffer_;

  Pallete pallete_;
  winrt::com_ptr<ID3D11Buffer> pallete_buffer_;

  DxCubeStereoRendererImpl(const winrt::com_ptr<ID3D11Device>& device)
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

    auto [vertices, indices, layouts] = Cube(false, true);

    auto vertex_buffer = grapho::dx11::CreateVertexBuffer(device_, vertices);
    instance_buffer_ = grapho::dx11::CreateVertexBuffer(
      device_, sizeof(Instance) * 65535, nullptr);
    auto index_buffer = grapho::dx11::CreateIndexBuffer(device_, indices);

    grapho::dx11::VertexSlot slots[]{
      { .VertexBuffer = vertex_buffer, .Stride = sizeof(Vertex) },
      { .VertexBuffer = instance_buffer_, .Stride = sizeof(Instance) },
    };
    drawable_ = grapho::dx11::Drawable::Create(
      device_, *vs, layouts, slots, index_buffer);
    assert(drawable_);

    constant_buffer_ = grapho::dx11::CreateConstantBuffer(
      device_, sizeof(DirectX::XMFLOAT4X4) * 2, nullptr);
    assert(constant_buffer_);

    pallete_buffer_ =
      grapho::dx11::CreateConstantBuffer(device_, sizeof(pallete_), &pallete_);
    assert(pallete_buffer_);
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

    ID3D11Buffer* vscb[]{ constant_buffer_.get() };
    context->VSSetConstantBuffers(0, 1, vscb);
    ID3D11Buffer* pscb[]{ pallete_buffer_.get() };
    context->PSSetConstantBuffers(0, 1, pscb);

    // 2
    drawable_->DrawInstance(context, CUBE_INDEX_COUNT, instanceCount);
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
