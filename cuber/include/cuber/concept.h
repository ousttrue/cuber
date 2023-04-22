#pragma once

namespace cuber {

template <typename T>
concept Float16 = sizeof(T) == sizeof(float) * 16;

template <typename T, typename A>
concept Float16_4 = requires(T, A) {
  sizeof(T) == sizeof(float) * 16;
  sizeof(A) == sizeof(float) * 4;
};

} // namespace cuber
