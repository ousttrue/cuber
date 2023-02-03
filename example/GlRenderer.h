#pragma once
#include <chrono>
#include <stdint.h>
#include <string_view>

using RenderTime = std::chrono::duration<float, std::ratio<1, 1>>;

// struct CameraMatrix {
//   float view[16] = {
//       1, 0, 0, 0, //
//       0, 1, 0, 0, //
//       0, 0, 1, 0, //
//       0, 0, 0, 1, //
//   };
//   float projection[16] = {
//   };
// };

class GlRenderer {
  struct GlRendererImpl *impl_ = nullptr;

public:
  GlRenderer(const GlRenderer &) = delete;
  GlRenderer &operator=(const GlRenderer &) = delete;
  GlRenderer();
  ~GlRenderer();

  void Load(std::string_view file);

  void RenderScene(RenderTime time, const float projection[16],
                   const float view[16]);
};
