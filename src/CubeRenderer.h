#pragma once
#include "geometry.h"
#include "shader.h"
#include "vao.h"
#include <memory>
namespace cuber {

union Instance {
  float matrix[16];
  struct {
    XYZW row0;
    XYZW row1;
    XYZW row2;
    XYZW row3;
  };
};

class CubeRenderer {
  std::shared_ptr<Vao> vao_;
  std::shared_ptr<ShaderProgram> shader_;
  std::shared_ptr<Vbo> instance_vbo_;

public:
  CubeRenderer(const CubeRenderer &) = delete;
  CubeRenderer &operator=(const CubeRenderer &) = delete;
  CubeRenderer();
  ~CubeRenderer();
  void Render(const float projection[16], const float view[16], std::span<Instance> instances);
};
} // namespace cuber