#include "mesh.h"

const float s = 0.5f;
XYZ positions[8] = {
    {-s, -s, +s}, //
    {-s, +s, +s}, //
    {+s, +s, +s}, //
    {+s, -s, +s}, //
    {-s, -s, -s}, //
    {-s, +s, -s}, //
    {+s, +s, -s}, //
    {+s, -s, -s}, //
};

//   5+-+6
//   / /|
// 1+-+2|
//  |4+-+7
//  |/ /
// 0+-+3
//
//   Y
//   A
//   +-> X
//  /
// L
//
struct Face {
  int indices[4];
  RGBA color;
};

// CCW
Face cube_faces[6] = {
    {
        .indices = {0, 3, 2, 1},
        .color = {1, 0, 0, 1},
    },
    {
        .indices = {3, 7, 6, 2},
        .color = {0, 1, 0, 1},
    },
    {
        .indices = {7, 4, 5, 6},
        .color = {0, 0, 1, 1},
    },
    {
        .indices = {4, 0, 1, 5},
        .color = {0, 1, 1, 1},
    },
    {
        .indices = {1, 2, 6, 5},
        .color = {1, 0, 1, 1},
    },
    {
        .indices = {3, 0, 4, 7},
        .color = {1, 1, 0, 1},
    },
};

struct Builder {
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
  void Quad(const XYZ &p0, const XYZ &p1, const XYZ &p2, const XYZ &p3,
            const RGBA &color) {
    // 01   00
    //  3+-+2
    //   | |
    //  0+-+1
    // 00   10
    Vertex v0{
        .position = p0,
        .barycentric = {1, 0},
    };
    Vertex v1{
        .position = p1,
        .barycentric = {0, 0},
    };
    Vertex v2{
        .position = p2,
        .barycentric = {0, 1},
    };
    Vertex v3{
        .position = p3,
        .barycentric = {0, 0},
    };
    auto index = vertices.size();
    vertices.push_back(v0);
    vertices.push_back(v1);
    vertices.push_back(v2);
    vertices.push_back(v3);
    // 0, 1, 2
    indices.push_back(index);
    indices.push_back(index + 1);
    indices.push_back(index + 2);
    // 2, 3, 0
    indices.push_back(index + 2);
    indices.push_back(index + 3);
    indices.push_back(index);
  }
};

Mesh Cube() {
  Builder builder;
  for (auto face : cube_faces) {
    builder.Quad(positions[face.indices[0]], positions[face.indices[1]],
                 positions[face.indices[2]], positions[face.indices[3]],
                 face.color);
  }
  return {
      builder.vertices,
      builder.indices,
  };
}