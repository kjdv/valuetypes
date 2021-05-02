#include <gtest/gtest.h>
#include <point/valuetypes.h>
#include <rapidcheck/gtest.h>
#include <unordered_set>

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

    if(x1 < x2) {
        unequal(p1, p2);
    } else if(x2 < x1) {
        unequal(p2, p1);
    } else if(y1 < y2) {
        unequal(p1, p2);
    } else if(y2 < y1) {
        unequal(p2, p1);
    } else {
        equal(p1, p2);
    }
}

RC_GTEST_PROP(Point, hashing, (double x1, double y1, double x2, double y2)) {
    vt::Point p1{x1, y1};
    vt::Point p2{x2, y2};

    auto h1 = std::hash<vt::Point>{}(p1);
    auto h2 = std::hash<vt::Point>{}(p2);

    if(p1 == p2) {
        RC_ASSERT(h1 == h2);
    } else {
        // failure is unlikely but possible, how to assert for that?
        RC_ASSERT(h1 != h2);
    }
}

TEST(Point, hashIsUsableForContainers) {
    vt::Point p1;
    vt::Point p2{1.0, 2.0};

    std::unordered_set<vt::Point> s;

    EXPECT_EQ(s.end(), s.find(p1));
    EXPECT_EQ(s.end(), s.find(p2));

    s.insert(p1);

    EXPECT_NE(s.end(), s.find(p1));
    EXPECT_EQ(s.end(), s.find(p2));

    s.insert(p2);

    EXPECT_NE(s.end(), s.find(p1));
    EXPECT_NE(s.end(), s.find(p2));
}

TEST(Point, insertion) {
    std::stringstream stream;
    vt::Point         p{1.0, 3.14};

    stream << p;
    EXPECT_EQ(R"({ "x": 1, "y": 3.14 })", stream.str());
}

RC_GTEST_PROP(Point, marshalling, (double x, double y)) {
    auto massage = [](double v) {
        std::stringstream s;
        s << v;
        double n;
        s >> n;
        return n;
    };

    vt::Point p1{massage(x), massage(y)};

    std::stringstream stream;
    stream << p1;

    vt::Point p2;
    stream >> p2;

    RC_ASSERT(p1 == p2);
}
