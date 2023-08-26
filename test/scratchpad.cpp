#include "scratchpad/valuetypes.h"
#include <gtest/gtest.h>
#include <rapidcheck/gtest.h>
#include <unordered_set>

namespace {

using namespace std;

auto make(const vector<string>& items) {
    sp::VectorTo target;
    transform(items.begin(), items.end(), back_inserter(target.v), [](auto v) {
        return sp::Nested{std::move(v)};
    });
    return target;
}

RC_GTEST_PROP(Scratchpad, totalOrdering, (string a1, string b1, string a2, string b2)) {
    sp::Compound c1{sp::Nested{a1}, sp::Nested{b1}};
    sp::Compound c2{sp::Nested{a2}, sp::Nested{b2}};

    auto unequal = [](const sp::Compound& smaller, const sp::Compound& larger) {
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
    auto equal = [](const sp::Compound& a, const sp::Compound& b) {
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

RC_GTEST_PROP(Scratchpad, hashing, (string a1, string b1, string a2, string b2)) {
    sp::Compound c1{sp::Nested{std::move(a1)}, sp::Nested{std::move(b1)}};
    sp::Compound c2{sp::Nested{std::move(a2)}, sp::Nested{std::move(b2)}};

    auto h1 = std::hash<sp::Compound>{}(c1);
    auto h2 = std::hash<sp::Compound>{}(c2);

    if(c1 == c2) {
        RC_ASSERT(h1 == h2);
    } else {
        // failure is unlikely but possible, how to assert for that?
        RC_ASSERT(h1 != h2);
    }
}

TEST(Scratchpad, hashIsUsableForContainers) {
    sp::Compound c1;
    sp::Compound c2{sp::Nested{"abc"}, sp::Nested{"def"}};

    std::unordered_set<sp::Compound> s;

    EXPECT_EQ(s.end(), s.find(c1));
    EXPECT_EQ(s.end(), s.find(c2));

    s.insert(c1);

    EXPECT_NE(s.end(), s.find(c1));
    EXPECT_EQ(s.end(), s.find(c2));

    s.insert(c2);

    EXPECT_NE(s.end(), s.find(c1));
    EXPECT_NE(s.end(), s.find(c2));
}

TEST(Scratchpad, extraction) {
    std::stringstream stream(R"({ "a": { "s": "abc" }, "b": { "s": "def" } })");
    sp::Compound      c;

    stream >> c;

    EXPECT_EQ("abc", c.a.s);
    EXPECT_EQ("def", c.b.s);
}

TEST(Scratchpad, extraction_with_extra) {
    std::stringstream stream(R"({ "a": { "s": "abc" }, "b": { "s": "def" }, "c": 123, "d": [456], "e": {"f": true} })");
    sp::Compound      c;

    stream >> c;

    EXPECT_EQ("abc", c.a.s);
    EXPECT_EQ("def", c.b.s);
}

RC_GTEST_PROP(Scratchpad, marshalling, (string a, string b)) {
    sp::Compound c1{sp::Nested{std::move(a)}, sp::Nested{std::move(b)}};

    std::stringstream stream;
    stream << c1;

    sp::Compound c2;
    stream >> c2;

    RC_ASSERT(c1 == c2);
}

RC_GTEST_PROP(ScratchpadV, totalOrdering, (vector<string> a, vector<string> b)) {
    sp::VectorTo v1 = make(a);
    sp::VectorTo v2 = make(b);

    auto unequal = [](const sp::VectorTo& smaller, const sp::VectorTo& larger) {
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
    auto equal = [](const sp::VectorTo& a, const sp::VectorTo& b) {
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

    if(a < b) {
        unequal(v1, v2);
    } else if(b < a) {
        unequal(v2, v1);
    } else {
        equal(v1, v2);
    }
}

RC_GTEST_PROP(ScratchpadV, hashing, (vector<string> a, vector<string> b)) {
    sp::VectorTo v1 = make(a);
    sp::VectorTo v2 = make(b);

    auto h1 = std::hash<sp::VectorTo>{}(v1);
    auto h2 = std::hash<sp::VectorTo>{}(v2);

    if(v1 == v2) {
        RC_ASSERT(h1 == h2);
    } else {
        // failure is unlikely but possible, how to assert for that?
        RC_ASSERT(h1 != h2);
    }
}

TEST(ScratchpadV, hashIsUsableForContainers) {
    sp::VectorTo v1;
    sp::VectorTo v2 = make({{"a", "b", "c"}});

    std::unordered_set<sp::VectorTo> s;

    EXPECT_EQ(s.end(), s.find(v1));
    EXPECT_EQ(s.end(), s.find(v2));

    s.insert(v1);

    EXPECT_NE(s.end(), s.find(v1));
    EXPECT_EQ(s.end(), s.find(v2));

    s.insert(v2);

    EXPECT_NE(s.end(), s.find(v1));
    EXPECT_NE(s.end(), s.find(v2));
}

TEST(ScratchpadV, swappable) {
    sp::VectorTo v1;
    sp::VectorTo v2 = make({{"a", "b", "c"}});

    std::swap(v1, v2);

    const std::vector<sp::Nested> expect{{"a"}, {"b"}, {"c"}};
    EXPECT_EQ(expect, v1.v);
}

TEST(ScratchpadV, extraction) {
    std::stringstream stream(R"({ "v": [ {"s": "a"}, {"s": "b"}, {"s": "c"}] })");
    sp::VectorTo      v;

    stream >> v;

    vector<sp::Nested> e{{"a"}, {"b"}, {"c"}};
    EXPECT_EQ(e, v.v);
}

TEST(ScratchpadV, optionalExtraction1) {
    std::stringstream   stream(R"({ "v": null })");
    sp::OptionalVectors v;

    stream >> v;
    EXPECT_FALSE(v.v);
}

TEST(ScratchpadV, optionalExtraction2) {
    std::stringstream   stream(R"({ "v": [ 1, null, 3 ] })");
    sp::OptionalVectors v;

    stream >> v;

    vector<optional<int>> e{1, optional<int>{}, 3};
    EXPECT_EQ(e, *v.v);
}

RC_GTEST_PROP(ScratchpadV, nestedMarshalling, (vector<string> a)) {
    sp::VectorTo v1 = make(a);

    std::stringstream stream;
    stream << v1;

    sp::VectorTo v2;
    stream >> v2;

    RC_ASSERT(v1 == v2);
}

} // namespace
