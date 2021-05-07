#include <gtest/gtest.h>
#include <rapidcheck/gtest.h>
#include <unordered_set>
#include <vectors/valuetypes.h>

namespace {

using namespace std;

RC_GTEST_PROP(Vectors, totalOrdering, (vector<int> a, vector<int> b)) {
    vt::Vectors v1{a};
    vt::Vectors v2{b};

    auto unequal = [](const vt::Vectors& smaller, const vt::Vectors& larger) {
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
    auto equal = [](const vt::Vectors& a, const vt::Vectors& b) {
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

RC_GTEST_PROP(Vectors, hashing, (vector<int> a, vector<int> b)) {
    vt::Vectors v1{move(a)};
    vt::Vectors v2{move(b)};

    auto h1 = std::hash<vt::Vectors>{}(v1);
    auto h2 = std::hash<vt::Vectors>{}(v2);

    if(v1 == v2) {
        RC_ASSERT(h1 == h2);
    } else {
        // failure is unlikely but possible, how to assert for that?
        RC_ASSERT(h1 != h2);
    }
}

TEST(Vectors, hashIsUsableForContainers) {
    vt::Vectors v1;
    vt::Vectors v2{{1, 2, 3}};

    std::unordered_set<vt::Vectors> s;

    EXPECT_EQ(s.end(), s.find(v1));
    EXPECT_EQ(s.end(), s.find(v2));

    s.insert(v1);

    EXPECT_NE(s.end(), s.find(v1));
    EXPECT_EQ(s.end(), s.find(v2));

    s.insert(v2);

    EXPECT_NE(s.end(), s.find(v1));
    EXPECT_NE(s.end(), s.find(v2));
}

TEST(Vectors, insertion) {
    std::stringstream stream;
    vt::Vectors       c{{1, 2, 3}};

    stream << c;
    EXPECT_EQ(R"({ "v": [ 1, 2, 3 ] })", stream.str());
}

RC_GTEST_PROP(Vectors, marshalling, (vector<int> a)) {
    vt::Vectors v1{move(a)};

    std::stringstream stream;
    stream << v1;

    vt::Vectors v2;
    stream >> v2;

    RC_ASSERT(v1 == v2);
}

} // namespace
