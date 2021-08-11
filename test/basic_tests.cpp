#include "test_base.h"

TEST(MathOpTests, IAddOpTest) { iadd_test(); }
TEST(MathOpTests, FAddOpTest) { fadd_test(); }
TEST(MathOpTests, ISubOpTest) { isub_test(); }
TEST(MathOpTests, FSubOpTest) { fsub_test(); }
TEST(MathOpTests, IMulOpTest) { imul_test(); }
TEST(MathOpTests, FMulOpTest) { fmul_test(); }
TEST(MathOpTests, IDivOpTest) { idiv_test(); }
TEST(MathOpTests, UDivOpTest) { udiv_test(); }
TEST(MathOpTests, FDivOpTest) { fdiv_test(); }
TEST(MathOpTests, IModOpTest) { imod_test(); }
TEST(MathOpTests, IMaxOpTest) { imax_test(); }
TEST(MathOpTests, UMaxOpTest) { umax_test(); }
TEST(MathOpTests, FMaxOpTest) { fmax_test(); }
TEST(MathOpTests, IMinOpTest) { imin_test(); }
TEST(MathOpTests, UMinOpTest) { umin_test(); }
TEST(MathOpTests, FMinOpTest) { fmin_test(); }
TEST(MathOpTests, INegOpTest) { ineg_test(); }
TEST(MathOpTests, FNegOpTest) { fneg_test(); }
TEST(MathOpTests, DNegOpTest) { dneg_test(); }
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

TEST(CastOpTests, SIToFPTest) { sitofp_test(); }
