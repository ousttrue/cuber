#pragma once
#include <memory>
#include <span>
#include <stdint.h>
#include <vector>

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

class Ibo {
  uint32_t ibo_ = 0;

public:
  Ibo(uint32_t ibo);
  ~Ibo();
  static std::shared_ptr<Ibo> Create(uint32_t size, const void *data);
  void Bind();
  void Unbind();
};

struct VertexLayout {
  uint32_t location;
  std::shared_ptr<Vbo> vbo;
  uint32_t type;
  uint32_t count;
  uint32_t offset;
  uint32_t stride;
};

class Vao {
  uint32_t vao_ = 0;
  std::vector<VertexLayout> layouts_;
  std::shared_ptr<Ibo> ibo_;
public:
  Vao(uint32_t vao, std::span<VertexLayout> layouts, const std::shared_ptr<Ibo> &ibo);
  ~Vao();
  static std::shared_ptr<Vao> Create(std::span<VertexLayout> layouts,
                                     const std::shared_ptr<Ibo> &ibo = {});
  void Bind();
  void Unbind();
  void Draw(uint32_t offset, uint32_t count);
};

} // namespace cuber