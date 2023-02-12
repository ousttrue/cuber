#include <cuber/mesh.h>

namespace cuber {

static VertexLayout layouts[] = {
    {
        .id =
            {
                .semantic_name = "POSITION",
                .semantic_index = 0,
            },
        .type = ValueType::Float,
        .count = 3,
        .offset = offsetof(Vertex, position),
        .stride = sizeof(Vertex),
    },
    {
        .id =
            {
                .semantic_name = "BARYCENTRIC",
                .semantic_index = 0,
            },
        .type = ValueType::Float,
        .count = 2,
        .offset = offsetof(Vertex, barycentric),
        .stride = sizeof(Vertex),
    },
    //
    {
        .id =
            {
                .semantic_name = "ROW",
                .semantic_index = 0,
            },
        .type = ValueType::Float,
        .count = 4,
        .offset = offsetof(Instance, row0),
        .stride = sizeof(Instance),
        .divisor = 1,
    },
    {
        .id =
            {
                .semantic_name = "ROW",
                .semantic_index = 1,
            },
        .type = ValueType::Float,
        .count = 4,
        .offset = offsetof(Instance, row1),
        .stride = sizeof(Instance),
        .divisor = 1,
    },
    {
        .id =
            {
                .semantic_name = "ROW",
                .semantic_index = 2,
            },
        .type = ValueType::Float,
        .count = 4,
        .offset = offsetof(Instance, row2),
        .stride = sizeof(Instance),
        .divisor = 1,
    },
    {
        .id =
            {
                .semantic_name = "ROW",
                .semantic_index = 3,
            },
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
  Mesh mesh_;
  bool ccw_;
  Builder(bool isCCW) : ccw_(isCCW) {}

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
    auto index = mesh_.vertices.size();
    mesh_.vertices.push_back(v0);
    mesh_.vertices.push_back(v1);
    mesh_.vertices.push_back(v2);
    mesh_.vertices.push_back(v3);
    if (ccw_) {
      // 0, 1, 2
      mesh_.indices.push_back(index);
      mesh_.indices.push_back(index + 1);
      mesh_.indices.push_back(index + 2);
      // 2, 3, 0
      mesh_.indices.push_back(index + 2);
      mesh_.indices.push_back(index + 3);
      mesh_.indices.push_back(index);
    } else {
      // 0, 3, 2
      mesh_.indices.push_back(index);
      mesh_.indices.push_back(index + 3);
      mesh_.indices.push_back(index + 2);
      // 2, 1, 0
      mesh_.indices.push_back(index + 2);
      mesh_.indices.push_back(index + 1);
      mesh_.indices.push_back(index);
    }
  }
};

Mesh Cube(bool isCCW, bool isStereo) {
  Builder builder(isCCW);
  for (auto layout : layouts) {
    layout.divisor *= (isStereo ? 2 : 1);
    builder.mesh_.layouts.push_back(layout);
  }
  for (auto face : cube_faces) {
    builder.Quad(positions[face.indices[0]], positions[face.indices[1]],
                 positions[face.indices[2]], positions[face.indices[3]],
                 face.color);
  }
  return builder.mesh_;
}

RGBA RED{0.8f, 0.2f, 0, 1};
RGBA DARK_RED{0.4f, 0, 0, 1};
RGBA BLUE{0, 0.4f, 0.8f, 1};
RGBA DARK_BLUE{0, 0, 0.4f, 1};
RGBA WHITE{0.8f, 0.8f, 0.9f, 1};

void PushGrid(std::vector<LineVertex> &lines, float interval, int half_count) {
  const float half = interval * half_count;
  for (int i = -half_count; i <= half_count; ++i) {
    if (i) {
      lines.push_back({
          .position = {-half, 0, static_cast<float>(i)},
          .color = WHITE,
      });
      lines.push_back({
          .position = {half, 0, static_cast<float>(i)},
          .color = WHITE,
      });
      lines.push_back({
          .position = {static_cast<float>(i), 0, -half},
          .color = WHITE,
      });
      lines.push_back({
          .position = {static_cast<float>(i), 0, half},
          .color = WHITE,
      });
    }
  }

  lines.push_back({
      .position = {half, 0, 0},
      .color = RED,
  });
  lines.push_back({
      .position = {0, 0, 0},
      .color = RED,
  });
  lines.push_back({
      .position = {-half, 0, 0},
      .color = DARK_RED,
  });
  lines.push_back({
      .position = {0, 0, 0},
      .color = DARK_RED,
  });

  lines.push_back({
      .position = {0, 0, half},
      .color = BLUE,
  });
  lines.push_back({
      .position = {0, 0, 0},
      .color = BLUE,
  });
  lines.push_back({
      .position = {0, 0, -half},
      .color = DARK_BLUE,
  });
  lines.push_back({
      .position = {0, 0, 0},
      .color = DARK_BLUE,
  });
}

} // namespace cuber