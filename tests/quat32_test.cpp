#include <catch2/catch_test_macros.hpp>
#include <muQuat32.h>
#include "../example/quat_packer.h"

TEST_CASE("quat32", "[pack, unpack]") {
  mu::quatf identity{0, 0, 0, 1};
  auto id32 = mu::quat32(identity);
  auto packed = quat_packer::pack(0, 0, 0, 1);
  REQUIRE(packed == (*((uint32_t *)&id32)));

  mu::quatf ex = id32;
  float unpacked[4];
  quat_packer::unpack(packed, unpacked);
  REQUIRE(ex.x == unpacked[0]);
  REQUIRE(ex.y == unpacked[1]);
  REQUIRE(ex.z == unpacked[2]);
  REQUIRE(ex.w == unpacked[3]);
}
