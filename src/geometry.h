#pragma once
#include <concepts>

namespace cuber {

struct xyz {
  float x;
  float y;
  float z;
};

struct xyzw {
  float x;
  float y;
  float z;
  float w;
};

struct rgba {
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