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

TEST(Scratchpad, extraction_simple) {
    std::stringstream stream(R"({ "s": "abc" })");
    sp::Nested        n;

    stream >> n;

    EXPECT_EQ("abc", n.s);
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

sp::Variants construct(int n, string s, int c) {
    sp::Variants v;
    if(c & 1) {
        v.v.emplace<string>(std::move(s));
    } else if(c & 2) {
        if(c & 4) {
            v.v.emplace<optional<sp::Nested>>();
        } else {
            v.v.emplace<optional<sp::Nested>>(sp::Nested{to_string(n + c)});
        }
    } else {
        v.v.emplace<int>(n);
    }

    return v;
}

RC_GTEST_PROP(ScratchpadVr, totalOrdering, (int n1, string s1, int n2, string s2, int c1, int c2)) {
    auto v1 = construct(n1, s1, c1);
    auto v2 = construct(n2, s2, c2);

    auto unequal = [](const sp::Variants& smaller, const sp::Variants& larger) {
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
    auto equal = [](const sp::Variants& a, const sp::Variants& b) {
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

RC_GTEST_PROP(ScratchpadVr, hashing, (int n1, string s1, int n2, string s2, int c1, int c2)) {
    auto v1 = construct(n1, s1, c1);
    auto v2 = construct(n2, s2, c2);

    auto h1 = std::hash<sp::Variants>{}(v1);
    auto h2 = std::hash<sp::Variants>{}(v2);

    if(v1 == v2) {
        RC_ASSERT(h1 == h2);
    } else {
        // failure is unlikely but possible, how to assert for that?
        RC_ASSERT(h1 != h2);
    }
}

TEST(ScratchpadVr, hashIsUsableForContainers) {
    sp::Variants v1;
    sp::Variants v2;
    v2.v.emplace<int>(1);

    std::unordered_set<sp::Variants> s;

    EXPECT_EQ(s.end(), s.find(v1));
    EXPECT_EQ(s.end(), s.find(v2));

    s.insert(v1);

    EXPECT_NE(s.end(), s.find(v1));
    EXPECT_EQ(s.end(), s.find(v2));

    s.insert(v2);

    EXPECT_NE(s.end(), s.find(v1));
    EXPECT_NE(s.end(), s.find(v2));
}

TEST(ScratchpadVr, extraction1) {
    std::stringstream stream(R"({ "v": { "int": 123 } })");
    sp::Variants      v;

    stream >> v;
    EXPECT_EQ(123, std::get<int>(v.v));
}

TEST(ScratchpadVr, extraction2) {
    std::stringstream stream(R"({ "v": { "custom_str": "abc" } })");
    sp::Variants      v;

    stream >> v;
    EXPECT_EQ("abc", std::get<std::string>(v.v));
}

TEST(ScratchpadVr, extraction3) {
    std::stringstream stream(R"({ "v": { "std::optional<Nested>": null } })");
    sp::Variants      v;

    stream >> v;
    EXPECT_FALSE(std::get<std::optional<sp::Nested>>(v.v));
}

TEST(ScratchpadVr, extraction4) {
    std::stringstream stream(R"({ "v": { "std::optional<Nested>": { "s": "abc" } } })");
    sp::Variants      v;

    stream >> v;
    EXPECT_EQ("abc", std::get<std::optional<sp::Nested>>(v.v)->s);
}

RC_GTEST_PROP(ScratchpadVr, marshalling, (int n, string s, int c)) {
    auto v1 = construct(n, std::move(s), c);

    std::stringstream stream;
    stream << v1;

    sp::Variants v2;
    stream >> v2;

    RC_ASSERT(v1 == v2);
}

} // namespace
