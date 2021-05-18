#include <gtest/gtest.h>
#include <rapidcheck/gtest.h>
#include <structs/valuetypes.h>
#include <unordered_set>

namespace {

using namespace std;

RC_GTEST_PROP(Structs, totalOrdering, (string a1, string b1, string a2, string b2)) {
    vt::Compound c1{vt::Nested{a1}, vt::Nested{b1}};
    vt::Compound c2{vt::Nested{a2}, vt::Nested{b2}};

    auto unequal = [](const vt::Compound& smaller, const vt::Compound& larger) {
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
    auto equal = [](const vt::Compound& a, const vt::Compound& b) {
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

    if(a1 < a2) {
        unequal(c1, c2);
    } else if(a2 < a1) {
        unequal(c2, c1);
    } else if(b1 < b2) {
        unequal(c1, c2);
    } else if(b2 < b1) {
        unequal(c2, c1);
    } else {
        equal(c1, c2);
    }
}

RC_GTEST_PROP(Structs, hashing, (string a1, string b1, string a2, string b2)) {
    vt::Compound c1{vt::Nested{move(a1)}, vt::Nested{move(b1)}};
    vt::Compound c2{vt::Nested{move(a2)}, vt::Nested{move(b2)}};

    auto h1 = std::hash<vt::Compound>{}(c1);
    auto h2 = std::hash<vt::Compound>{}(c2);

    if(c1 == c2) {
        RC_ASSERT(h1 == h2);
    } else {
        // failure is unlikely but possible, how to assert for that?
        RC_ASSERT(h1 != h2);
    }
}

TEST(Structs, hashIsUsableForContainers) {
    vt::Compound c1;
    vt::Compound c2{vt::Nested{"abc"}, vt::Nested{"def"}};

    std::unordered_set<vt::Compound> s;

    EXPECT_EQ(s.end(), s.find(c1));
    EXPECT_EQ(s.end(), s.find(c2));

    s.insert(c1);

    EXPECT_NE(s.end(), s.find(c1));
    EXPECT_EQ(s.end(), s.find(c2));

    s.insert(c2);

    EXPECT_NE(s.end(), s.find(c1));
    EXPECT_NE(s.end(), s.find(c2));
}

TEST(Structs, extraction) {
    std::stringstream stream(R"({ "a": { "s": "abc" }, "b": { "s": "def" } })");
    vt::Compound      c;

    stream >> c;

    EXPECT_EQ("abc", c.a.s);
    EXPECT_EQ("def", c.b.s);
}

RC_GTEST_PROP(Structs, marshalling, (string a, string b)) {
    vt::Compound c1{vt::Nested{move(a)}, vt::Nested{move(b)}};

    std::stringstream stream;
    stream << c1;

    vt::Compound c2;
    stream >> c2;

    RC_ASSERT(c1 == c2);
}

} // namespace
