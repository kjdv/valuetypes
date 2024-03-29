#include <gtest/gtest.h>
#include <optionals/valuetypes.h>
#include <rapidcheck/gtest.h>
#include <sstream>
#include <unordered_set>

namespace vt {
namespace {

using namespace std;

TEST(Optionals, construction) {
    Optionals o1{};

    EXPECT_FALSE(o1.b);
    EXPECT_FALSE(o1.n);
    EXPECT_FALSE(o1.x);
    EXPECT_FALSE(o1.s);

    Optionals o2{true, 1, 3.14, "abc"};

    EXPECT_TRUE(o2.b);
    EXPECT_EQ(1, o2.n);
    EXPECT_EQ(3.14, o2.x);
    EXPECT_EQ("abc", o2.s);
}

TEST(Optionals, json) {
    istringstream stream1(R"({ "b": null, "n": null, "x": null, "s": null })");
    istringstream stream2(R"({ "b": true, "n": 1, "x": 2, "s": "abc" })");
    Optionals     e1{};
    Optionals     e2{true, 1, 2.0, "abc"};

    Optionals a1{true}, a2;
    stream1 >> a1;
    stream2 >> a2;

    EXPECT_EQ(e1, a1);
    EXPECT_EQ(e2, a2);
}

TEST(BasicTypes, hashing) {
    Optionals o1{};
    Optionals o2{true, 1, 2.0, "abc"};

    auto h1 = std::hash<Optionals>{}(o1);
    auto h2 = std::hash<Optionals>{}(o2);

    EXPECT_NE(h1, h2);
}

TEST(BasicTypes, hashIsUsableForContainers) {
    Optionals o1{};
    Optionals o2{true, 1, 2.0, "abc"};

    std::unordered_set<Optionals> s;

    EXPECT_EQ(s.end(), s.find(o1));
    EXPECT_EQ(s.end(), s.find(o2));

    s.insert(o1);

    EXPECT_NE(s.end(), s.find(o1));
    EXPECT_EQ(s.end(), s.find(o2));

    s.insert(o2);

    EXPECT_NE(s.end(), s.find(o1));
    EXPECT_NE(s.end(), s.find(o2));
}

RC_GTEST_PROP(Optionals, marshalling, (bool b, int n, double x, string s, int coin)) {
    Optionals o1;
    if(coin & 1) {
        o1.b = b;
    }
    if(coin & 2) {
        o1.n = n;
    }
    if(coin & 4) {
        o1.x = x;
    }
    if(coin & 8) {
        o1.s = s;
    }

    std::stringstream stream;
    stream << o1;

    Optionals o2;
    stream >> o2;

    RC_ASSERT(o1 == o2);
}

} // namespace
} // namespace vt
