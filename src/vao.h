#pragma once
#include <memory>
#include <span>
#include <stdint.h>

namespace cuber {

class Vbo {
  uint32_t vbo_ = 0;

public:
  Vbo(uint32_t vbo);
  ~Vbo();
  static std::shared_ptr<Vbo> Create(uint32_t size, const void *data);
  void Bind();
  void Unbind();
};

struct VertexLayout {
  uint32_t type;
  uint32_t count;
  uint32_t offset;
  uint32_t stride;
};

class Vao {
  uint32_t vao_ = 0;
  std::shared_ptr<Vbo> vbo_;

public:
  Vao(uint32_t vao, const std::shared_ptr<Vbo> vbo);
  ~Vao();
  static std::shared_ptr<Vao> Create(const std::shared_ptr<Vbo> &vbo,
                                     std::span<VertexLayout> layouts);
  void Bind();
  void Unbind();
  void Draw(uint32_t offset, uint32_t count);
};

} // namespace cuber