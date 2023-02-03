#pragma once
#include "geometry.h"
#include "shader.h"
#include "vao.h"
#include <memory>
namespace cuber {

struct Cube {
  xyz position = {0, 0, 0};
  xyzw rotation = {0, 0, 0, 1};
};

class CubeRenderer {
  std::shared_ptr<Vao> vao_;
  std::shared_ptr<ShaderProgram> shader_;

  std::vector<Cube> cubes_;

public:
  CubeRenderer(const CubeRenderer &) = delete;
  CubeRenderer &operator=(const CubeRenderer &) = delete;
  CubeRenderer();
  ~CubeRenderer();
  void Push(float size, const xyz &pos);
  void Render(const float projection[16], const float view[16]);
};
} // namespace cuber