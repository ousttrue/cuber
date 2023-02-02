#pragma once
#include <numbers>

class TurnTable {
  float fovY_ = std::numbers::pi * 30.0f / 180.0f;
  int width_ = 1.0f;
  int height_ = 1.0f;
  float nearZ_ = 0.01f;
  float farZ_ = 100.0f;

  float yaw_ = 0;
  float pitch_ = 0;
  float shift_[3] = {0, 0, -5};

public:
  void SetSize(int w, int h);
  void Update(float projection[16], float view[16]) const;
};
