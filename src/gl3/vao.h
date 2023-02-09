#pragma once
#include <memory>
#include <mesh.h>
#include <span>
#include <stdint.h>
#include <vector>

namespace cuber {

class Vbo {
  uint32_t vbo_ = 0;

public:
  Vbo(uint32_t vbo);
  ~Vbo();
  static std::shared_ptr<Vbo> Create(uint32_t size, const void *data = nullptr);
  void Bind();
  void Unbind();
  void Upload(uint32_t size, const void *data);
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

struct VertexSlot
{
  uint32_t location;
  std::shared_ptr<Vbo> vbo;
};

class Vao {
  uint32_t vao_ = 0;
  std::vector<VertexLayout> layouts_;
  std::shared_ptr<Ibo> ibo_;

public:
  Vao(uint32_t vao, std::span<VertexLayout> layouts, std::span<VertexSlot> slots,
      const std::shared_ptr<Ibo> &ibo);
  ~Vao();
  static std::shared_ptr<Vao> Create(std::span<VertexLayout> layouts, std::span<VertexSlot> slots,
                                     const std::shared_ptr<Ibo> &ibo = {});
  void Bind();
  void Unbind();
  void Draw(uint32_t count, uint32_t offset = 0);
  void DrawInstance(uint32_t primcount, uint32_t count, uint32_t offset = 0);
};

} // namespace cuber