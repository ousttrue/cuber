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
  uint32_t vertex_buffer = 0;
  uint32_t vertex_shader = 0;
  uint32_t fragment_shader = 0;
  uint32_t program = 0;
  uint32_t mvp_location = -1;
  uint32_t vpos_location = -1;
  uint32_t vcol_location = -1;

public:
  GlRenderer(const GlRenderer &) = delete;
  GlRenderer &operator=(const GlRenderer &) = delete;
  GlRenderer();
  ~GlRenderer();

  void Load(std::string_view file);

  void RenderScene(RenderTime time, const float projection[16],
                   const float view[16]);
};
