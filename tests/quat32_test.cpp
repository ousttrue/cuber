#include "../example/quat_packer.h"
#include <catch2/catch_test_macros.hpp>
#include <muQuat32.h>

TEST_CASE("pack, unpack", "[quat32]") {
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

TEST_CASE("bitfield", "[quat32]") {
  {
    quat_packer::Packed packed{};
    REQUIRE(packed.value() == 0);
  }
  {
    quat_packer::Packed packed{
        .x0 = 1,
    };
    REQUIRE(packed.value() == 1);
  }
  {
    quat_packer::Packed packed{
        .x1 = 1,
    };
    REQUIRE(packed.value() == 0x400);
  }
  {
    quat_packer::Packed packed{
        .x2 = 1,
    };
    REQUIRE(packed.value() == 0x100000);
  }
  {
    quat_packer::Packed packed{
        .drop = 1,
    };
    REQUIRE(packed.value() == 0x40000000);
  }
}
