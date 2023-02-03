#pragma once
#include "error_handler.h"
#include "geometry.h"
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <stdint.h>
#include <string_view>

namespace cuber {

class ShaderProgram {
  uint32_t program_ = 0;

  ShaderProgram(uint32_t program);

public:
  ~ShaderProgram();
  static std::shared_ptr<ShaderProgram> Create(const ErrorHandler &onError,
                                               std::span<std::string_view> vs,
                                               std::span<std::string_view> fs);
  void Bind();
  void Unbind();
  std::optional<uint32_t> AttributeLocation(const char *name);
  void _SetUniformMatrix(const ErrorHandler &onError, const char *name,
                         const float m[16]);
  template <Mat4 T>
  void SetUniformMatrix(const ErrorHandler &onError, const char *name,
                        const T &t) {
    _SetUniformMatrix(onError, name, (float *)&t);
  }
};
} // namespace cuber
