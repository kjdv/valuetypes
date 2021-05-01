#include <gtest/gtest.h>
#include <internal_sample.hh>

namespace valuetypes {
namespace {

TEST(sample, private_function) {
    EXPECT_EQ("hello from a private function", private_function());
}

} // namespace
} // namespace valuetypes
