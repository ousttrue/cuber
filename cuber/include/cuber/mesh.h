#pragma once
#include <GL/glew.h>

#include <grapho/mesh.h>
#include <string>
#include <vector>

namespace cuber {
const int CUBE_INDEX_COUNT = 36;

grapho::Mesh Cube(bool isCCW, bool isStereo);

void PushGrid(std::vector<grapho::LineVertex> &lines, float interval = 1.0f,
              int half_count = 5);

} // namespace cuber
