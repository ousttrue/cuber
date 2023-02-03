#pragma once
#include <concepts>

namespace cuber {

struct xyz {
  float x = 0;
  float y = 0;
  float z = 0;
};

struct rgba {
  float r = 1;
  float g = 1;
  float b = 1;
  float a = 1;
};

template <typename T>
concept Mat4 = requires(T &) {
  sizeof(T) == 4 * 16;
};

} // namespace cuber