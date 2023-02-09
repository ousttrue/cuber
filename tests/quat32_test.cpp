#include <gtest/gtest.h>

#include "../example/bvhutil/srht.h"
#include <muQuat32.h>

TEST(quat32, packunpack) {
  mu::quatf identity{0, 0, 0, 1};
  auto id32 = mu::quat32(identity);
  auto packed = quat_packer::Pack(0, 0, 0, 1);
  ASSERT_EQ(packed, (*((uint32_t *)&id32)));

  mu::quatf ex = id32;
  float unpacked[4];
  quat_packer::Unpack(packed, unpacked);
  ASSERT_EQ(ex.x, unpacked[0]);
  ASSERT_EQ(ex.y, unpacked[1]);
  ASSERT_EQ(ex.z, unpacked[2]);
  ASSERT_EQ(ex.w, unpacked[3]);
}

TEST(quat32, bitfield) {
  {
    quat_packer::Packed packed{};
    ASSERT_EQ(packed.value, 0);
  }
  {
    quat_packer::Packed packed{
        .x0 = 1,
    };
    ASSERT_EQ(packed.value, 1);
  }
  {
    quat_packer::Packed packed{
        .x1 = 1,
    };
    ASSERT_EQ(packed.value, 0x400);
  }
  {
    quat_packer::Packed packed{
        .x2 = 1,
    };
    ASSERT_EQ(packed.value, 0x100000);
  }
  {
    quat_packer::Packed packed{
        .drop = 1,
    };
    ASSERT_EQ(packed.value, 0x40000000);
  }

  {
    quat_packer::Packed packed{
        .value = (uint32_t)-1,
    };
    packed.x0 = 0;
    ASSERT_EQ(packed.value, 0xfffffc00);
  }
  {
    quat_packer::Packed packed{
        .value = (uint32_t)-1,
    };
    packed.x1 = 0;
    ASSERT_EQ(packed.value, 0xfff003ff);
  }
  {
    quat_packer::Packed packed{
        .value = (uint32_t)-1,
    };
    packed.x2 = 0;
    ASSERT_EQ(packed.value, 0xc00fffff);
  }
  {
    quat_packer::Packed packed{
        .value = (uint32_t)-1,
    };
    packed.drop = 0;
    ASSERT_EQ(packed.value, 0x3fffffff);
  }
}
