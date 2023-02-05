#pragma once
#include <concepts>

namespace cuber {

struct XY {
  float x;
  float y;
};

struct XYZ {
  float x;
  float y;
  float z;
};

struct XYZW {
  float x;
  float y;
  float z;
  float w;
};

struct RGBA {
  float r;
  float g;
  float b;
  float a;
};

template <typename T>
concept Mat4 = requires(T &) {
  sizeof(T) == 4 * 16;
};

} // namespace cuber