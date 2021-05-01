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

TEST(point, equality) {
    Point orig{1.1, 2.2};
    Point same{1.1, 2.2};
    Point diff1{1.2, 2.2};
    Point diff2{1.1, 2.1};

    EXPECT_TRUE(orig == same);
    EXPECT_FALSE(orig != same);

    EXPECT_FALSE(orig == diff1);
    EXPECT_TRUE(orig != diff1);

    EXPECT_FALSE(orig == diff2);
    EXPECT_TRUE(orig != diff2);
}
