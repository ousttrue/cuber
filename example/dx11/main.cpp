#include "BvhPanel.h"
#include "DxPlatform.h"
#include "GuiApp.h"
#include <cuber/dx/DxCubeRenderer.h>
#include <cuber/dx/DxLineRenderer.h>
#include <grapho/dx11/texture.h>

struct rgba
{
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t a;
};

int
main(int argc, char** argv)
{
  // imgui
  GuiApp app;

  DxPlatform platform;
  if (!platform.Create()) {
    return 1;
  }

  // bvh scene
  BvhPanel bvhPanel;

  // load bvh
  if (argc > 1) {
    if (auto bvh = Bvh::ParseFile(argv[1])) {
      bvhPanel.SetBvh(bvh);
    }
  }

  // renderer
  auto device = platform.GetDevice();
  cuber::dx11::DxCubeRenderer cubeRenderer(device);
  // pallete
  const auto PalleteIndex = 7;
  const auto TextureBind = 0;
  cubeRenderer.Pallete.Colors[PalleteIndex] = { 1, 1, 1, 1 };
  cubeRenderer.Pallete.Textures[PalleteIndex] = {
    TextureBind, TextureBind, TextureBind, 0
  };
  cubeRenderer.UploadPallete();

  cuber::dx11::DxLineRenderer lineRenderer(device);
  std::vector<cuber::LineVertex> lines;
  cuber::PushGrid(lines);

  // texture
  std::vector<cuber::Instance> instances;
  instances.push_back({});
  auto t = DirectX::XMMatrixTranslation(0, 1, -1);
  auto s = DirectX::XMMatrixScaling(1.6f, 0.9f, 0.1f);
  DirectX::XMStoreFloat4x4(&instances.back().Matrix, s * t);

  static rgba pixels[4] = {
    { 255, 0, 0, 255 },
    { 0, 255, 0, 255 },
    { 0, 0, 255, 255 },
    { 255, 255, 255, 255 },
  };
  auto texture = grapho::dx11::Texture::Create(device, 2, 2, &pixels[0].r);

  // main loop
  winrt::com_ptr<ID3D11DeviceContext> context;
  device->GetImmediateContext(context.put());
  while (auto time = platform.NewFrame(app.clear_color)) {
    // imgui
    {
      app.UpdateGui();
      bvhPanel.UpdateGui();
    }
    auto data = app.RenderGui();

    // scene
    {
      auto cubes = bvhPanel.GetCubes();
      instances.resize(1 + cubes.size());
      std::copy(cubes.begin(), cubes.end(), instances.data() + 1);
      texture->Bind(context, 0);
      texture->Bind(context, 1);
      texture->Bind(context, 2);
      texture->Bind(context, 3);
      cubeRenderer.Render(
        app.projection, app.view, instances.data(), instances.size());
      lineRenderer.Render(app.projection, app.view, lines);
      platform.EndFrame(data);
    }
  }

  return 0;
}
