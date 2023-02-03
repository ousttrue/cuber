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

//
// Vao
//
Vao::Vao(uint32_t vao, const std::shared_ptr<Vbo> vbo) : vao_(vao), vbo_(vbo) {}
Vao::~Vao() {}
std::shared_ptr<Vao> Vao::Create(const std::shared_ptr<Vbo> &vbo,
                                 std::span<VertexLayout> layouts) {
  GLuint vao;
  glGenVertexArrays(1, &vao);
  auto ptr = std::shared_ptr<Vao>(new Vao(vao, vbo));
  ptr->Bind();
  vbo->Bind();
  for (int i = 0; i < layouts.size(); ++i) {
    glEnableVertexAttribArray(i);
    auto &layout = layouts[i];
    glVertexAttribPointer(i, layout.count, layout.type, GL_FALSE, layout.stride,
                          (void *)layout.offset);
  }
  vbo->Unbind();
  ptr->Unbind();
  return ptr;
}
void Vao::Bind() { glBindVertexArray(vao_); }
void Vao::Unbind() { glBindVertexArray(0); }
void Vao::Draw(uint32_t offset, uint32_t count) {
  Bind();
  glDrawArrays(GL_TRIANGLES, offset, count);
  Unbind();
}

} // namespace cuber