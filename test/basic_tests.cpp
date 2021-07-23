#include "test_base.h"
#include "gtest/gtest.h"

TEST(MathOpTests, IAddOpTest) { iadd_test(); }
TEST(MathOpTests, FAddOpTest) { fadd_test(); }
TEST(MathOpTests, ISubOpTest) { isub_test(); }
TEST(MathOpTests, FSubOpTest) { fsub_test(); }
