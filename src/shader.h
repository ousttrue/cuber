#pragma once
#include <functional>
#include <memory>
#include <span>
#include <stdint.h>
#include <string_view>

namespace cuber {
class ShaderProgram {
  uint32_t program_ = 0;

  ShaderProgram(uint32_t program);

public:
  ~ShaderProgram();
  static std::shared_ptr<ShaderProgram>
  Create(const std::function<void(std::string_view)> &onError,
         std::span<std::string_view> vs, std::span<std::string_view> fs);
};
} // namespace cuber
