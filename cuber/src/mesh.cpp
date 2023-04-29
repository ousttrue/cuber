#include <cuber/mesh.h>

using namespace grapho;

namespace cuber {

static VertexLayout layouts[] = {
    {
        .Id =
            {
                .AttributeLocation = 0,
                .Slot= 0,
                .SemanticName = "POSITION",
                .SemanticIndex = 0,
            },
        .Type = ValueType::Float,
        .Count = 4,
        .Offset = offsetof(Vertex, Position),
        .Stride = sizeof(Vertex),
    },
    {
        .Id =
            {
                .AttributeLocation = 1,
                .Slot = 0,
                .SemanticName = "TEXCOORD",
                .SemanticIndex = 0,
            },
        .Type = ValueType::Float,
        .Count = 4,
        .Offset = offsetof(Vertex, UvBarycentric),
        .Stride = sizeof(Vertex),
    },
    //
    {
        .Id =
            {
                .AttributeLocation = 2,
                .Slot = 1,
                .SemanticName = "ROW",
                .SemanticIndex = 0,
            },
        .Type = ValueType::Float,
        .Count = 4,
        .Offset = offsetof(Instance, Row0),
        .Stride = sizeof(Instance),
        .Divisor = 1,
    },
    {
        .Id =
            {
                .AttributeLocation = 3,
                .Slot = 1,
                .SemanticName = "ROW",
                .SemanticIndex = 1,
            },
        .Type = ValueType::Float,
        .Count = 4,
        .Offset = offsetof(Instance, Row1),
        .Stride = sizeof(Instance),
        .Divisor = 1,
    },
    {
        .Id =
            {
                .AttributeLocation = 4,
                .Slot = 1,
                .SemanticName = "ROW",
                .SemanticIndex = 2,
            },
        .Type = ValueType::Float,
        .Count = 4,
        .Offset = offsetof(Instance, Row2),
        .Stride = sizeof(Instance),
        .Divisor = 1,
    },
    {
        .Id =
            {
                .AttributeLocation = 5,
                .Slot = 1,
                .SemanticName = "ROW",
                .SemanticIndex = 3,
            },
        .Type = ValueType::Float,
        .Count = 4,
        .Offset = offsetof(Instance, Row3),
        .Stride = sizeof(Instance),
        .Divisor = 1,
    },
    //
    {
        .Id =
            {
                .AttributeLocation = 6,
                .Slot = 1,
                .SemanticName = "COLOR",
                .SemanticIndex = 0,
            },
        .Type = ValueType::Float,
        .Count = 4,
        .Offset = offsetof(Instance, Color),
        .Stride = sizeof(Instance),
        .Divisor = 1,
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
    layout.Divisor *= (isStereo ? 2 : 1);
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
