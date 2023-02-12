#pragma once

namespace cuber {

template <typename T>
concept Float16 = sizeof(T) == sizeof(float) * 16;

}
