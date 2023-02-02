#include "TurnTable.h"
#include <DirectXMath.h>

void TurnTable::SetSize(int w, int h) {
  if (w == width_ && h == height_) {
    return;
  }
  width_ = w;
  height_ = h;
}

void TurnTable::Update(float projection[16], float view[16]) const {
  float aspectRatio = (float)width_ / (float)height_;
  auto p =
      DirectX::XMMatrixPerspectiveFovRH(fovY_, aspectRatio, nearZ_, farZ_);
  DirectX::XMStoreFloat4x4((DirectX::XMFLOAT4X4 *)projection, p);

  auto yaw = DirectX::XMMatrixRotationY(yaw_);
  auto pitch = DirectX::XMMatrixRotationX(pitch_);
  auto shift = DirectX::XMMatrixTranslation(shift_[0], shift_[1], shift_[2]);
  DirectX::XMStoreFloat4x4((DirectX::XMFLOAT4X4 *)view, yaw * pitch * shift);
}
