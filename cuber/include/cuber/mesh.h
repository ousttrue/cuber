#pragma once
#include <grapho/vertexlayout.h>
#include <vector>

namespace cuber {
const int CUBE_INDEX_COUNT = 36;

struct Vertex
{
  grapho::XMFLOAT4 PositionFace;
  grapho::XMFLOAT4 UvBarycentric;
};

struct Mesh
{
  std::vector<Vertex> Vertices;
  std::vector<uint32_t> Indices;
  std::vector<grapho::VertexLayout> Layouts;
};

Mesh
Cube(bool isCCW, bool isStereo);

enum class ColorName : uint8_t
{
  Error, // magenta
  Red,
  Green,
  Blue,
  DarkRed,
  DarkGreen,
  DarkBlue,
};

struct Pallete
{
  static constexpr grapho::XMFLOAT4 Red = { 1, 0, 0, 1 };
  static constexpr grapho::XMFLOAT4 Green = { 0, 1, 0, 1 };
  static constexpr grapho::XMFLOAT4 Blue = { 0, 0, 1, 1 };
  static constexpr grapho::XMFLOAT4 DarkRed = { 0.5f, 0, 0, 1 };
  static constexpr grapho::XMFLOAT4 DarkGreen = { 0, 0.5f, 0, 1 };
  static constexpr grapho::XMFLOAT4 DarkBlue = { 0, 0, 0.5f, 1 };
  static constexpr grapho::XMFLOAT4 Magenta = { 1, 0, 1, 1 };
  static constexpr grapho::XMFLOAT4 White{ 0.8f, 0.8f, 0.9f, 1 };
  static constexpr grapho::XMFLOAT4 Black{ 0, 0, 0, 1 };

  grapho::XMFLOAT4 Colors[32]{
    // error
    Magenta,
    //
    Red,
    Green,
    Blue,
    DarkRed,
    DarkGreen,
    DarkBlue,
    White,
    Black,
  };
  grapho::XMFLOAT4 Textures[32]{
    // error
    { -1, -1, -1, -1 },
    // no texture
    { -1, -1, -1, -1 }, // Red
    { -1, -1, -1, -1 }, // Green
    { -1, -1, -1, -1 }, // Blue
    { -1, -1, -1, -1 }, // DarkRed
    { -1, -1, -1, -1 }, // DarkGreen
    { -1, -1, -1, -1 }, // DarkBlue
    { -1, -1, -1, -1 }, // White
    { -1, -1, -1, -1 }, // Black
  };
};
static_assert(sizeof(Pallete) == 1024, "Pallete");

struct Instance
{
  union
  {
    struct
    {
      grapho::XMFLOAT4 Row0;
      grapho::XMFLOAT4 Row1;
      grapho::XMFLOAT4 Row2;
      grapho::XMFLOAT4 Row3;
    };
    grapho::XMFLOAT4X4 Matrix;
  };
  grapho::XMFLOAT4 PositiveFaceFlag = { 1, 2, 3, 0 };
  grapho::XMFLOAT4 NegativeFaceFlag = { 4, 5, 6, 0 };
};
static_assert(sizeof(Instance) == 96, "sizeof Instance");

struct LineVertex
{
  grapho::XMFLOAT3 Position;
  grapho::XMFLOAT4 Color;
};

void
PushGrid(std::vector<LineVertex>& lines,
         float interval = 1.0f,
         int half_count = 5);

}
