#include <gtest/gtest.h>
#include <point/valuetypes.hh>

TEST(point, constructable) {
    Point p1;
    EXPECT_EQ(0.0, p1.x);
    EXPECT_EQ(0.0, p1.y);

    Point p2{1.0, -3.1};
    EXPECT_EQ(1.0, p2.x);
    EXPECT_EQ(-3.1, p2.y);
}
