#pragma once
#include <GL/glew.h>

#include <DirectXMath.h>
#include <grapho/mesh.h>
#include <string>
#include <vector>

namespace cuber {
const int CUBE_INDEX_COUNT = 36;

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
  DirectX::XMFLOAT4 Color;
};

grapho::Mesh
Cube(bool isCCW, bool isStereo);

void
PushGrid(std::vector<grapho::LineVertex>& lines,
         float interval = 1.0f,
         int half_count = 5);

} // namespace cuber
