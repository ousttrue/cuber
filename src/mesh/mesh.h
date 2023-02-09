#pragma once
#include <vector>
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

struct Mesh {
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
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

Mesh Cube();
