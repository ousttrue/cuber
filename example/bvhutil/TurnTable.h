#pragma once
#include <numbers>

class TurnTable {
  float fovY_ = static_cast<float>(std::numbers::pi * 30.0f / 180.0f);
  int width_ = 1;
  int height_ = 1;
  float nearZ_ = 0.01f;
  float farZ_ = 1000.0f;

  float yaw_ = 0;
  float pitch_ = 0;
  float shift_[3] = {0, -1, -4};

public:
  TurnTable(bool leftHanded) {
    if (leftHanded) {
      // shift_[2] = -shift_[2];
    }
  }
  void SetSize(int w, int h);
  void YawPitch(int dx, int dy);
  void Shift(int dx, int dy);
  void Dolly(int d);
  void Update(float projection[16], float view[16]) const;
};
