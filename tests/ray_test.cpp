#include <DirectXCollision.h>
#include <gtest/gtest.h>

TEST(DirectXMath, ray)
{
  auto origin = DirectX::XMVectorSet(0, 0, 40, 1);
  auto dir = DirectX::XMVectorSet(0, 0, -1, 0);

  {
    float dist;
    auto test =
      DirectX::TriangleTests::Intersects(origin,
                                         dir,
                                         DirectX::XMVectorSet(0, 0, -1, 1),
                                         DirectX::XMVectorSet(1, 0, -1, 1),
                                         DirectX::XMVectorSet(0, 1, -1, 1),
                                         dist);
    EXPECT_TRUE(test);
    EXPECT_EQ(dist, 41);
  }
  {
    // flip
    float dist;
    auto test =
      DirectX::TriangleTests::Intersects(origin,
                                         dir,
                                         DirectX::XMVectorSet(0, 0, -1, 1),
                                         DirectX::XMVectorSet(0, 1, -1, 1),
                                         DirectX::XMVectorSet(1, 0, -1, 1),
                                         dist);
    EXPECT_TRUE(test);
    EXPECT_EQ(dist, 41);
  }
}
