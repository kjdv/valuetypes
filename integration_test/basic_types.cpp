#include <gtest/gtest.h>
#include <basic_types/valuetypes.h>
#include <rapidcheck/gtest.h>
#include <sstream>

namespace bt {
namespace {

using namespace std;

TEST(BasicTypes, construction) {
    BasicTypes bt{true, 123, 3.14};

    EXPECT_TRUE(bt.truth);
    EXPECT_EQ(123, bt.n);
    EXPECT_EQ(3.14, bt.x);
}

TEST(BasicTypes, json) {
    ostringstream stream;
    BasicTypes bt{};
    stream << bt;

    auto expect = R"({ "truth": false, "n": 0, "x": 0, "s": "" })";
    EXPECT_EQ(expect, stream.str());
}

RC_GTEST_PROP(BasicTypes, marshalling, (bool truth, int n, double x, string s)) {
    BasicTypes bt1{truth, n, x, move(s)};

    std::stringstream stream;
    stream << bt1;

    BasicTypes bt2;
    stream >> bt2;

    RC_ASSERT(bt1 == bt2);
}

}
}
