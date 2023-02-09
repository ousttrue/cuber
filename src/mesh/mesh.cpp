#include "mesh.h"

static VertexLayout layouts[] = {
    {
        .semantic_name = "POSITION",
        .semantic_index = 0,
        .type = ValueType::Float,
        .count = 3,
        .offset = offsetof(Vertex, position),
        .stride = sizeof(Vertex),
    },
    {
        .semantic_name = "BARYCENTRIC",
        .semantic_index = 0,
        .type = ValueType::Float,
        .count = 2,
        .offset = offsetof(Vertex, barycentric),
        .stride = sizeof(Vertex),
    },
    //
    {
        .semantic_name = "ROW",
        .semantic_index = 0,
        .type = ValueType::Float,
        .count = 4,
        .offset = offsetof(Instance, row0),
        .stride = sizeof(Instance),
        .divisor = 1,
    },
    {
        .semantic_name = "ROW",
        .semantic_index = 1,
        .type = ValueType::Float,
        .count = 4,
        .offset = offsetof(Instance, row1),
        .stride = sizeof(Instance),
        .divisor = 1,
    },
    {
        .semantic_name = "ROW",
        .semantic_index = 2,
        .type = ValueType::Float,
        .count = 4,
        .offset = offsetof(Instance, row2),
        .stride = sizeof(Instance),
        .divisor = 1,
    },
    {
        .semantic_name = "ROW",
        .semantic_index = 3,
        .type = ValueType::Float,
        .count = 4,
        .offset = offsetof(Instance, row3),
        .stride = sizeof(Instance),
        .divisor = 1,
    },
};

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
  Mesh mesh;
  bool ccw;
  Builder(bool isCCW) : ccw(isCCW) {}

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
    auto index = mesh.vertices.size();
    mesh.vertices.push_back(v0);
    mesh.vertices.push_back(v1);
    mesh.vertices.push_back(v2);
    mesh.vertices.push_back(v3);
    if (ccw) {
      // 0, 1, 2
      mesh.indices.push_back(index);
      mesh.indices.push_back(index + 1);
      mesh.indices.push_back(index + 2);
      // 2, 3, 0
      mesh.indices.push_back(index + 2);
      mesh.indices.push_back(index + 3);
      mesh.indices.push_back(index);
    } else {
      // 0, 3, 2
      mesh.indices.push_back(index);
      mesh.indices.push_back(index + 3);
      mesh.indices.push_back(index + 2);
      // 2, 1, 0
      mesh.indices.push_back(index + 2);
      mesh.indices.push_back(index + 1);
      mesh.indices.push_back(index);
    }
  }
};

Mesh Cube(bool isCCW) {
  Builder builder(isCCW);
  builder.mesh.layouts.assign(layouts, layouts + std::size(layouts));
  for (auto face : cube_faces) {
    builder.Quad(positions[face.indices[0]], positions[face.indices[1]],
                 positions[face.indices[2]], positions[face.indices[3]],
                 face.color);
  }
  return builder.mesh;
}