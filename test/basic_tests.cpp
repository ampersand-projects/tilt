#include "test_base.h"

TEST(MathOpTests, AddOpTest) { add_test(); }
TEST(MathOpTests, SubOpTest) { sub_test(); }
TEST(MathOpTests, MulOpTest) { mul_test(); }
TEST(MathOpTests, DivOpTest) { div_test(); }
TEST(MathOpTests, ModOpTest) { mod_test(); }
TEST(MathOpTests, MaxOpTest) { max_test(); }
TEST(MathOpTests, MinOpTest) { min_test(); }
TEST(MathOpTests, NegOpTest) { neg_test(); }
TEST(MathOpTests, SqrtOpTest) { sqrt_test(); }
TEST(MathOpTests, PowOPTest) { pow_test(); }
TEST(MathOpTests, CeilOPTest) { ceil_test(); }
TEST(MathOpTests, FloorOPTest) { floor_test(); }
TEST(MathOpTests, AbsOPTest) { abs_test(); }
TEST(CastOpTests, CastTest) { cast_test(); }
TEST(QuiltTest, MovingSumTest) { moving_sum_test(); }
TEST(QuiltTest, NormTest) { norm_test(); }
TEST(QuiltTest, ResampleTest1) { resample_test(5, 4); }
TEST(QuiltTest, ResampleTest2) { resample_test(6, 3); }
TEST(QuiltTest, ResampleTest3) { resample_test(4, 5); }
TEST(QuiltTest, ResampleTest4) { resample_test(3, 6); }
