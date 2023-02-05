#include <catch2/catch_test_macros.hpp>
#include <muQuat32.h>

static constexpr float SR2 = 1.41421356237f;
static constexpr float RSR2 = 1.0f / 1.41421356237f;
static constexpr float C = float(0x3ff);
static constexpr float R = 1.0f / float(0x3ff);

static constexpr uint32_t pack(float a) {
  return static_cast<uint32_t>((a * SR2 + 1.0f) * 0.5f * C);
}
static constexpr float unpack(uint32_t a) {
  return ((a * R) * 2.0f - 1.0f) * RSR2;
}
static constexpr float square(float a) { return a * a; }
static int dropmax(float a, float b, float c, float d) {
  if (a > b && a > c && a > d)
    return 0;
  if (b > c && b > d)
    return 1;
  if (c > d)
    return 2;
  return 3;
}
static float sign(float v) { return v < float(0.0) ? float(-1.0) : float(1.0); }

static uint32_t pack(float x, float y, float z, float w) {

  struct {
    uint32_t x0 : 10;
    uint32_t x1 : 10;
    uint32_t x2 : 10;
    uint32_t drop : 2;
  } value;

  float a0, a1, a2;
  value.drop = dropmax(square(x), square(y), square(z), square(w));
  if (value.drop == 0) {
    float s = sign(x);
    a0 = y * s;
    a1 = z * s;
    a2 = w * s;
  } else if (value.drop == 1) {
    float s = sign(y);
    a0 = x * s;
    a1 = z * s;
    a2 = w * s;
  } else if (value.drop == 2) {
    float s = sign(z);
    a0 = x * s;
    a1 = y * s;
    a2 = w * s;
  } else {
    float s = sign(w);
    a0 = x * s;
    a1 = y * s;
    a2 = z * s;
  }

  value.x0 = pack(a0);
  value.x1 = pack(a1);
  value.x2 = pack(a2);

  return *(uint32_t *)&value;
}

TEST_CASE("quat32", "[pack]") {
  mu::quatf identity{0, 0, 0, 1};
  auto id32 = mu::quat32(identity);
  REQUIRE((pack(0, 0, 0, 1) >> 2) == (*((uint32_t *)&id32) >> 2));
  //   mu::quatf ex = id32;
  //   REQUIRE(identity == ex);
}
