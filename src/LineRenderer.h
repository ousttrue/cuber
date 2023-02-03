#pragma once
#include "geometry.h"

namespace cuber {
struct Vertex {
  xyz position;
  rgba color;
};
class LineRenderer {
public:
  LineRenderer();
  ~LineRenderer();
  void PushLine(const xyz &start, const xyz &end);
  void PushGrid(float size);
  void Render(const float projection[16], const float view[16]);
};
} // namespace cuber