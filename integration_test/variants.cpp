#include <algorithm>
#include <gtest/gtest.h>
#include <rapidcheck/gtest.h>
#include <unordered_set>
#include <variants/valuetypes.h>

namespace {

using namespace std;

vt::Variants construct(int n, string s, int c) {
    vt::Variants v;
    if(c & 1) {
        v.v.emplace<string>(move(s));
    } else if(c & 2) {
        if(c & 4) {
            v.v.emplace<optional<vt::Base>>();
        } else {
            v.v.emplace<optional<vt::Base>>(vt::Base{n + c});
        }
    } else {
        v.v.emplace<int>(n);
    }

    return v;
}

RC_GTEST_PROP(Variants, totalOrdering, (int n1, string s1, int n2, string s2, int c1, int c2)) {
    auto v1 = construct(n1, s1, c1);
    auto v2 = construct(n2, s2, c2);

    auto unequal = [](const vt::Variants& smaller, const vt::Variants& larger) {
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
    auto equal = [](const vt::Variants& a, const vt::Variants& b) {
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

    if(v1 < v2) {
        unequal(v1, v2);
    } else if(v2 < v1) {
        unequal(v2, v1);
    } else {
        equal(v1, v2);
    }
}

RC_GTEST_PROP(Variants, hashing, (int n1, string s1, int n2, string s2, int c1, int c2)) {
    auto v1 = construct(n1, s1, c1);
    auto v2 = construct(n2, s2, c2);

    auto h1 = std::hash<vt::Variants>{}(v1);
    auto h2 = std::hash<vt::Variants>{}(v2);

    if(v1 == v2) {
        RC_ASSERT(h1 == h2);
    } else {
        // failure is unlikely but possible, how to assert for that?
        RC_ASSERT(h1 != h2);
    }
}

TEST(Variants, hashIsUsableForContainers) {
    vt::Variants v1;
    vt::Variants v2;
    v2.v.emplace<int>(1);

    std::unordered_set<vt::Variants> s;

    EXPECT_EQ(s.end(), s.find(v1));
    EXPECT_EQ(s.end(), s.find(v2));

    s.insert(v1);

    EXPECT_NE(s.end(), s.find(v1));
    EXPECT_EQ(s.end(), s.find(v2));

    s.insert(v2);

    EXPECT_NE(s.end(), s.find(v1));
    EXPECT_NE(s.end(), s.find(v2));
}

TEST(Variants, extraction1) {
    std::stringstream stream(R"({ "v": { "int": 123 } })");
    vt::Variants      v;

    stream >> v;
    EXPECT_EQ(123, std::get<int>(v.v));
}

TEST(Variants, extraction2) {
    std::stringstream stream(R"({ "v": { "custom_str": "abc" } })");
    vt::Variants      v;

    stream >> v;
    EXPECT_EQ("abc", std::get<std::string>(v.v));
}

TEST(Variants, extraction3) {
    std::stringstream stream(R"({ "v": { "std::optional<Base>": null } })");
    vt::Variants      v;

    stream >> v;
    EXPECT_FALSE(std::get<std::optional<vt::Base>>(v.v));
}

TEST(Variants, extraction4) {
    std::stringstream stream(R"({ "v": { "std::optional<Base>": { "n": 123 } } })");
    vt::Variants      v;

    stream >> v;
    EXPECT_EQ(123, std::get<std::optional<vt::Base>>(v.v)->n);
}

RC_GTEST_PROP(Variants, marshalling, (int n, string s, int c)) {
    auto v1 = construct(n, move(s), c);

    std::stringstream stream;
    stream << v1;

    vt::Variants v2;
    stream >> v2;

    RC_ASSERT(v1 == v2);
}

} // namespace
