#pragma once
#include <DirectXMath.h>
#include <grapho/vertexlayout.h>
#include <string>
#include <vector>

namespace cuber {
const int CUBE_INDEX_COUNT = 36;

struct Vertex
{
  DirectX::XMFLOAT4 PositionFace;
  DirectX::XMFLOAT4 UvBarycentric;
};

struct Mesh
{
  std::vector<Vertex> Vertices;
  std::vector<uint32_t> Indices;
  std::vector<grapho::VertexLayout> Layouts;
};

Mesh
Cube(bool isCCW, bool isStereo);

struct byte4
{
  uint8_t x;
  uint8_t y;
  uint8_t z;
  uint8_t w;
};

struct Instance
{
  union
  {
    struct
    {
      DirectX::XMFLOAT4 Row0;
      DirectX::XMFLOAT4 Row1;
      DirectX::XMFLOAT4 Row2;
      DirectX::XMFLOAT4 Row3;
    };
    DirectX::XMFLOAT4X4 Matrix;
  };
  DirectX::XMFLOAT4 PositiveFaceFlag;
  DirectX::XMFLOAT4 NegativeFaceFlag;
};

struct LineVertex
{
  DirectX::XMFLOAT3 Position;
  DirectX::XMFLOAT4 Color;
};

void
PushGrid(std::vector<LineVertex>& lines,
         float interval = 1.0f,
         int half_count = 5);

}
