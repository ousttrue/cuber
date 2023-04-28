#include <cuber/mesh.h>

using namespace grapho;

namespace cuber {

static VertexLayout layouts[] = {
    {
        .id =
            {
                .semantic_name = "POSITION",
                .semantic_index = 0,
            },
        .type = ValueType::Float,
        .count = 4,
        .offset = offsetof(Vertex, Position),
        .stride = sizeof(Vertex),
    },
    {
        .id =
            {
                .semantic_name = "TEXCOORD",
                .semantic_index = 0,
            },
        .type = ValueType::Float,
        .count = 4,
        .offset = offsetof(Vertex, UvBarycentric),
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
        .offset = offsetof(Instance, Row0),
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
        .offset = offsetof(Instance, Row1),
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
        .offset = offsetof(Instance, Row2),
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
        .offset = offsetof(Instance, Row3),
        .stride = sizeof(Instance),
        .divisor = 1,
    },
    //
    {
        .id =
            {
                .semantic_name = "COLOR",
                .semantic_index = 0,
            },
        .type = ValueType::Float,
        .count = 4,
        .offset = offsetof(Instance, Color),
        .stride = sizeof(Instance),
        .divisor = 1,
    },
};

const float s = 0.5f;
DirectX::XMFLOAT4 positions[8] = {
  { -s, -s, +s, 1 }, //
  { -s, +s, +s, 1 }, //
  { +s, +s, +s, 1 }, //
  { +s, -s, +s, 1 }, //
  { -s, -s, -s, 1 }, //
  { -s, +s, -s, 1 }, //
  { +s, +s, -s, 1 }, //
  { +s, -s, -s, 1 }, //
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
struct Face
{
  int Indices[4];
  DirectX::XMFLOAT4 Color;
};

// CCW
Face cube_faces[6] = {
  {
    .Indices = { 0, 3, 2, 1 },
    .Color = { 1, 0, 0, 1 },
  },
  {
    .Indices = { 3, 7, 6, 2 },
    .Color = { 0, 1, 0, 1 },
  },
  {
    .Indices = { 7, 4, 5, 6 },
    .Color = { 0, 0, 1, 1 },
  },
  {
    .Indices = { 4, 0, 1, 5 },
    .Color = { 0, 1, 1, 1 },
  },
  {
    .Indices = { 1, 2, 6, 5 },
    .Color = { 1, 0, 1, 1 },
  },
  {
    .Indices = { 3, 0, 4, 7 },
    .Color = { 1, 1, 0, 1 },
  },
};

struct Builder
{
  Mesh Mesh;
  bool CCW;
  Builder(bool isCCW)
    : CCW(isCCW)
  {
  }

  void Quad(const DirectX::XMFLOAT4& p0,
            const DirectX::XMFLOAT4& p1,
            const DirectX::XMFLOAT4& p2,
            const DirectX::XMFLOAT4& p3,
            const DirectX::XMFLOAT4& color)
  {
    // 01   00
    //  3+-+2
    //   | |
    //  0+-+1
    // 00   10
    Vertex v0{
      .Position = p0,
      .UvBarycentric = { 0, 1, 1, 0 },
    };
    Vertex v1{
      .Position = p1,
      .UvBarycentric = { 1, 1, 0, 0 },
    };
    Vertex v2{
      .Position = p2,
      .UvBarycentric = { 1, 0, 0, 1 },
    };
    Vertex v3{
      .Position = p3,
      .UvBarycentric = { 0, 0, 0, 0 },
    };
    auto index = Mesh.Vertices.size();
    Mesh.Vertices.push_back(v0);
    Mesh.Vertices.push_back(v1);
    Mesh.Vertices.push_back(v2);
    Mesh.Vertices.push_back(v3);
    if (CCW) {
      // 0, 1, 2
      Mesh.Indices.push_back(index);
      Mesh.Indices.push_back(index + 1);
      Mesh.Indices.push_back(index + 2);
      // 2, 3, 0
      Mesh.Indices.push_back(index + 2);
      Mesh.Indices.push_back(index + 3);
      Mesh.Indices.push_back(index);
    } else {
      // 0, 3, 2
      Mesh.Indices.push_back(index);
      Mesh.Indices.push_back(index + 3);
      Mesh.Indices.push_back(index + 2);
      // 2, 1, 0
      Mesh.Indices.push_back(index + 2);
      Mesh.Indices.push_back(index + 1);
      Mesh.Indices.push_back(index);
    }
  }
};

Mesh
Cube(bool isCCW, bool isStereo)
{
  Builder builder(isCCW);
  for (auto layout : layouts) {
    layout.divisor *= (isStereo ? 2 : 1);
    builder.Mesh.Layouts.push_back(layout);
  }
  for (auto face : cube_faces) {
    builder.Quad(positions[face.Indices[0]],
                 positions[face.Indices[1]],
                 positions[face.Indices[2]],
                 positions[face.Indices[3]],
                 face.Color);
  }
  return builder.Mesh;
}

DirectX::XMFLOAT4 RED{ 0.8f, 0.2f, 0, 1 };
DirectX::XMFLOAT4 DARK_RED{ 0.4f, 0, 0, 1 };
DirectX::XMFLOAT4 BLUE{ 0, 0.4f, 0.8f, 1 };
DirectX::XMFLOAT4 DARK_BLUE{ 0, 0, 0.4f, 1 };
DirectX::XMFLOAT4 WHITE{ 0.8f, 0.8f, 0.9f, 1 };

void
PushGrid(std::vector<LineVertex>& lines, float interval, int half_count)
{
  const float half = interval * half_count;
  for (int i = -half_count; i <= half_count; ++i) {
    if (i) {
      lines.push_back({
        .Position = { -half, 0, static_cast<float>(i) },
        .Color = WHITE,
      });
      lines.push_back({
        .Position = { half, 0, static_cast<float>(i) },
        .Color = WHITE,
      });
      lines.push_back({
        .Position = { static_cast<float>(i), 0, -half },
        .Color = WHITE,
      });
      lines.push_back({
        .Position = { static_cast<float>(i), 0, half },
        .Color = WHITE,
      });
    }
  }

  lines.push_back({
    .Position = { half, 0, 0 },
    .Color = RED,
  });
  lines.push_back({
    .Position = { 0, 0, 0 },
    .Color = RED,
  });
  lines.push_back({
    .Position = { -half, 0, 0 },
    .Color = DARK_RED,
  });
  lines.push_back({
    .Position = { 0, 0, 0 },
    .Color = DARK_RED,
  });

  lines.push_back({
    .Position = { 0, 0, half },
    .Color = BLUE,
  });
  lines.push_back({
    .Position = { 0, 0, 0 },
    .Color = BLUE,
  });
  lines.push_back({
    .Position = { 0, 0, -half },
    .Color = DARK_BLUE,
  });
  lines.push_back({
    .Position = { 0, 0, 0 },
    .Color = DARK_BLUE,
  });
}

} // namespace cuber
