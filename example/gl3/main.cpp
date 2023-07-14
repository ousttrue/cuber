#include <GL/glew.h>

#include "BvhPanel.h"
#include "GlfwPlatform.h"
#include "GuiApp.h"
#include <cuber/gl3/GlCubeRenderer.h>
#include <cuber/gl3/GlLineRenderer.h>
#include <grapho/gl3/texture.h>

struct rgba
{
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t a;
};

const auto TextureBind = 0;
const auto PalleteIndex = 7;

int
main(int argc, char** argv)
{
  // imgui
  GuiApp app;
  app.Camera.Transform.Translation = {0, 1, 4};

  // window
  GlfwPlatform platform;
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

  // cuber
  cuber::gl3::GlCubeRenderer cubeRenderer;
  cuber::gl3::GlLineRenderer lineRenderer;

  std::vector<cuber::LineVertex> lines;
  cuber::PushGrid(lines);

  // texture
  static rgba pixels[4] = {
    { 255, 0, 0, 255 },
    { 0, 255, 0, 255 },
    { 0, 0, 255, 255 },
    { 255, 255, 255, 255 },
  };
  auto texture = grapho::gl3::Texture::Create({
    .Width = 2,
    .Height = 2,
    .Format = grapho::PixelFormat::u8_RGBA,
    .Pixels = &pixels[0].r,
  });
  texture->SamplingPoint();

  std::vector<cuber::Instance> instances;
  instances.push_back({});
  auto t = DirectX::XMMatrixTranslation(0, 1, -1);
  auto s = DirectX::XMMatrixScaling(1.6f, 0.9f, 0.1f);
  DirectX::XMStoreFloat4x4(&instances.back().Matrix, s * t);
  instances.back().PositiveFaceFlag = {
    PalleteIndex, PalleteIndex, PalleteIndex, 0
  };
  instances.back().NegativeFaceFlag = {
    PalleteIndex, PalleteIndex, PalleteIndex, 0
  };
  cubeRenderer.Pallete.Colors[PalleteIndex] = { 1, 1, 1, 1 };
  cubeRenderer.Pallete.Textures[PalleteIndex] = {
    TextureBind, TextureBind, TextureBind, TextureBind
  };
  cubeRenderer.UploadPallete();

  // main loop
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
      texture->Activate(TextureBind);
      cubeRenderer.Render(&app.Camera.ProjectionMatrix._11,
                          &app.Camera.ViewMatrix._11,
                          instances.data(),
                          instances.size());
      lineRenderer.Render(
        &app.Camera.ProjectionMatrix._11, &app.Camera.ViewMatrix._11, lines);
      platform.EndFrame(data);
    }
  }

  return 0;
}
