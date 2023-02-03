#pragma once
#include "geometry.h"
namespace cuber {
class CubeRenderer {
public:
  CubeRenderer(const CubeRenderer &) = delete;
  CubeRenderer &operator=(const CubeRenderer &) = delete;
  CubeRenderer();
  ~CubeRenderer();
  void Push(float size, const xyz &pos);
  void Render(const float projection[16], const float view[16]);
};
} // namespace cuber