#include <basic_types/valuetypes.h>
#include <gtest/gtest.h>
#include <rapidcheck/gtest.h>
#include <sstream>
#include <unordered_set>

namespace bt {
namespace {

using namespace std;

TEST(BasicTypes, construction) {
    BasicTypes bt{true, 123, 3.14};

    EXPECT_TRUE(bt.truth);
    EXPECT_EQ(123, bt.n);
    EXPECT_EQ(3.14, bt.x);
}

TEST(BasicTypes, constructWithDefaults) {
    WithDefaults wd{};

    EXPECT_TRUE(wd.b);
    EXPECT_EQ(123, wd.n);
    EXPECT_EQ(3.14, wd.x);
    EXPECT_EQ("abc", wd.s);
    EXPECT_EQ(456, wd.o);
}

TEST(BasicTypes, json) {
    istringstream stream(R"({
    "truth": true,
    "n": 1,
    "x": 2.0,
    "s": "abc"
})");
    BasicTypes    bt;
    stream >> bt;

    EXPECT_EQ(true, bt.truth);
    EXPECT_EQ(1, bt.n);
    EXPECT_EQ(2.0, bt.x);
    EXPECT_EQ("abc", bt.s);
}

RC_GTEST_PROP(BasicTypes, hashing, (bool truth1, int n1, double x1, string s1, bool truth2, int n2, double x2, string s2)) {
    BasicTypes bt1{truth1, n1, x1, std::move(s1)};
    BasicTypes bt2{truth2, n2, x2, std::move(s2)};

    auto h1 = std::hash<BasicTypes>{}(bt1);
    auto h2 = std::hash<BasicTypes>{}(bt2);

    if(bt1 == bt2) {
        RC_ASSERT(h1 == h2);
    } else {
        // failure is unlikely but possible, how to assert for that?
        RC_ASSERT(h1 != h2);
    }
}

TEST(BasicTypes, hashIsUsableForContainers) {
    BasicTypes bt1;
    BasicTypes bt2{true, 123, 3.14, "abc"};

    std::unordered_set<BasicTypes> s;

    EXPECT_EQ(s.end(), s.find(bt1));
    EXPECT_EQ(s.end(), s.find(bt2));

    s.insert(bt1);

    EXPECT_NE(s.end(), s.find(bt1));
    EXPECT_EQ(s.end(), s.find(bt2));

    s.insert(bt2);

    EXPECT_NE(s.end(), s.find(bt1));
    EXPECT_NE(s.end(), s.find(bt2));
}

RC_GTEST_PROP(BasicTypes, marshalling, (bool truth, int n, double x, string s)) {
    BasicTypes bt1{truth, n, x, std::move(s)};

    std::stringstream stream;
    stream << bt1;

    BasicTypes bt2;
    stream >> bt2;

    RC_ASSERT(bt1 == bt2);
}

} // namespace
} // namespace bt
