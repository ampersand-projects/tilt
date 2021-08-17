#include "test_base.h"

TEST(MathOpTests, IAddOpTest) { add_test(); }
TEST(MathOpTests, ISubOpTest) { sub_test(); }
TEST(MathOpTests, IMulOpTest) { mul_test(); }
TEST(MathOpTests, IDivOpTest) { div_test(); }
TEST(MathOpTests, IModOpTest) { mod_test(); }
TEST(MathOpTests, IMaxOpTest) { max_test(); }
TEST(MathOpTests, IMinOpTest) { min_test(); }
TEST(MathOpTests, DNegOpTest) { neg_test(); }
TEST(MathOpTests, FSqrtOpTest) { sqrt_test(); }
TEST(MathOpTests, FPowOPTest) { pow_test(); }
TEST(MathOpTests, FCeilOPTest) { ceil_test(); }
TEST(MathOpTests, FFloorOPTest) { floor_test(); }
TEST(MathOpTests, FAbsOPTest) { abs_test(); }
TEST(CastOpTests, Int8ToInt32Test) { cast_test(); }
TEST(StatefulOpTest, MovingSumTest) { moving_sum_test(); }
