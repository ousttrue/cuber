#pragma once
#include <gl/glew.h>

#include <grapho/mesh.h>
#include <string>
#include <vector>

namespace cuber {
const int CUBE_INDEX_COUNT = 36;

struct XY {
  float x;
  float y;
};

struct XYZ {
  float x;
  float y;
  float z;
};

struct Vertex {
  XYZ position;
  XY barycentric;
};
static_assert(sizeof(Vertex) == sizeof(float) * 5, "Vertex");

struct Mesh {
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
  std::vector<grapho::VertexLayout> layouts;

  Mesh() {}
  ~Mesh() {}
};

struct XYZW {
  float x;
  float y;
  float z;
  float w;
};

struct RGBA {
  float r;
  float g;
  float b;
  float a;
};

struct Instance {
  XYZW row0;
  XYZW row1;
  XYZW row2;
  XYZW row3;
};

Mesh Cube(bool isCCW, bool isStereo);

struct LineVertex {
  XYZ position;
  RGBA color;
};
static_assert(sizeof(LineVertex) == 28);

void PushGrid(std::vector<LineVertex> &lines, float interval = 1.0f,
              int half_count = 5);

} // namespace cuber
