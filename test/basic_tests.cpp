#include "test_base.h"
#include "gtest/gtest.h"

TEST(MathOpTests, IAddOpTest) { iadd_test(); }
TEST(MathOpTests, FAddOpTest) { fadd_test(); }
TEST(MathOpTests, ISubOpTest) { isub_test(); }
TEST(MathOpTests, FSubOpTest) { fsub_test(); }
TEST(MathOpTests, FSqrtOpTest) { fsqrt_test(); }
TEST(MathOpTests, DSqrtOpTest) { dsqrt_test(); }
TEST(MathOpTests, FPowOPTest) { fpow_test(); }
TEST(MathOpTests, DPowOPTest) { dpow_test(); }
TEST(MathOpTests, FCeilOPTest) { fceil_test(); }
TEST(MathOpTests, DCeilOPTest) { dceil_test(); }
TEST(MathOpTests, FFloorOPTest) { ffloor_test(); }
TEST(MathOpTests, DFloorOPTest) { dfloor_test(); }
TEST(MathOpTests, FAbsOPTest) { fabs_test(); }
TEST(MathOpTests, DAbsOPTest) { dabs_test(); }
TEST(MathOpTests, IAbsOPTest) { iabs_test(); }
