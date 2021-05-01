#include <gtest/gtest.h>
#include <point/valuetypes.hh>
#include <rapidcheck/gtest.h>

TEST(Point, constructable) {
    vt::Point p1;
    EXPECT_EQ(0.0, p1.x);
    EXPECT_EQ(0.0, p1.y);

    vt::Point p2{1.0, -3.1};
    EXPECT_EQ(1.0, p2.x);
    EXPECT_EQ(-3.1, p2.y);
}

RC_GTEST_PROP(Point, totalOrdering, (double x1, double y1, double x2, double y2)) {
    vt::Point p1{x1, y1};
    vt::Point p2{x2, y2};

    auto unequal = [](vt::Point smaller, vt::Point larger) {
        RC_ASSERT(smaller < larger);
        RC_ASSERT(smaller <= larger);
        RC_ASSERT(!(smaller > larger));
        RC_ASSERT(!(smaller >= larger));
        RC_ASSERT(!(larger < smaller));
        RC_ASSERT(!(larger <= smaller));
        RC_ASSERT(larger > smaller);
        RC_ASSERT(larger >= smaller);
        RC_ASSERT(!(smaller == larger));
        RC_ASSERT(smaller != larger);
        RC_ASSERT(!(larger == smaller));
        RC_ASSERT(larger != smaller);
    };
    auto equal = [](vt::Point a, vt::Point b) {
        RC_ASSERT(!(a < b));
        RC_ASSERT(a <= b);
        RC_ASSERT(!(a > b));
        RC_ASSERT(a >= b);
        RC_ASSERT(!(b < a));
        RC_ASSERT(b <= a);
        RC_ASSERT(!(b > a));
        RC_ASSERT(b >= a);
        RC_ASSERT(a == b);
        RC_ASSERT(b == a);
        RC_ASSERT(!(a != b));
        RC_ASSERT(!(b != a));
    };

    if (x1 < x2) {
        unequal(p1, p2);
    } else if (x2 < x1) {
        unequal(p2, p1);
    } else if (y1 < y2) {
        unequal(p1, p2);
    } else if (y2 < y1) {
        unequal(p2, p1);
    } else {
        equal(p1, p2);
    }
}
