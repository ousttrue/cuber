#include "vao.h"
#include <GL/glew.h>

namespace cuber {

//
// Vbo
//
Vbo::Vbo(uint32_t vbo) : vbo_(vbo) {}
Vbo::~Vbo() { glDeleteBuffers(1, &vbo_); }
std::shared_ptr<Vbo> Vbo::Create(uint32_t size, const void *data) {
  GLuint vbo;
  glGenBuffers(1, &vbo);
  auto ptr = std::shared_ptr<Vbo>(new Vbo(vbo));
  ptr->Bind();
  if (data) {
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
  } else {
    glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
  }
  ptr->Unbind();
  return ptr;
}
void Vbo::Bind() { glBindBuffer(GL_ARRAY_BUFFER, vbo_); }
void Vbo::Unbind() { glBindBuffer(GL_ARRAY_BUFFER, 0); }
void Vbo::Upload(uint32_t size, const void *data) {
  Bind();
  glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
  Unbind();
}

//
// Ibo
//
Ibo::Ibo(uint32_t ibo) : ibo_(ibo) {}
Ibo::~Ibo() {}
std::shared_ptr<Ibo> Ibo::Create(uint32_t size, const void *data) {
  GLuint ibo;
  glGenBuffers(1, &ibo);
  auto ptr = std::shared_ptr<Ibo>(new Ibo(ibo));
  ptr->Bind();
  if (data) {
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
  } else {
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
  }
  ptr->Unbind();
  return ptr;
}
void Ibo::Bind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_); }
void Ibo::Unbind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }

//
// Vao
//
Vao::Vao(uint32_t vao, std::span<VertexLayout> layouts,
         const std::shared_ptr<Ibo> &ibo)
    : vao_(vao), layouts_(layouts.begin(), layouts.end()), ibo_(ibo) {
  Bind();
  if (ibo_) {
    ibo_->Bind();
  }
  for (int i = 0; i < layouts.size(); ++i) {
    auto &layout = layouts[i];
    glEnableVertexAttribArray(layout.location);
    layout.vbo->Bind();
    glVertexAttribPointer(
        layout.location, layout.count, layout.type, GL_FALSE, layout.stride,
        reinterpret_cast<void *>(static_cast<uint64_t>(layout.offset)));
    if (layout.divisor) {
      auto a = glVertexAttribDivisor;
      glVertexAttribDivisor(layout.location, layout.divisor);
    }
  }
  Unbind();
  if (ibo_) {
    ibo_->Unbind();
  }
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}
Vao::~Vao() {}
std::shared_ptr<Vao> Vao::Create(std::span<VertexLayout> layouts,
                                 const std::shared_ptr<Ibo> &ibo) {
  GLuint vao;
  glGenVertexArrays(1, &vao);
  auto ptr = std::shared_ptr<Vao>(new Vao(vao, layouts, ibo));
  return ptr;
}
void Vao::Bind() { glBindVertexArray(vao_); }
void Vao::Unbind() { glBindVertexArray(0); }
void Vao::Draw(uint32_t count, uint32_t offset) {
  Bind();
  if (ibo_) {
    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT,
                   reinterpret_cast<void *>(static_cast<uint64_t>(offset)));
  } else {
    glDrawArrays(GL_TRIANGLES, offset, count);
  }
  Unbind();
}
void Vao::DrawInstance(uint32_t primcount, uint32_t count, uint32_t offset) {
  Bind();
  glDrawElementsInstanced(
      GL_TRIANGLES, count, GL_UNSIGNED_INT,
      reinterpret_cast<void *>(static_cast<uint64_t>(offset)), primcount);
  Unbind();
}

} // namespace cuber