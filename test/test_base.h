#ifndef TEST_TEST_BASE_H_
#define TEST_TEST_BASE_H_

#include <functional>
#include <vector>

#include "tilt/ir/op.h"

#include "gtest/gtest.h"

using namespace std;
using namespace tilt;

template<typename T>
struct Event {
    int64_t st;
    int64_t et;
    T payload;
};

template<typename InTy, typename OutTy>
using QueryFn = function<vector<Event<OutTy>>(vector<Event<InTy>>)>;

template<typename InTy, typename OutTy>
void op_test(Op, QueryFn<InTy, OutTy>, vector<Event<InTy>>);

template<typename InTy, typename OutTy>
void select_test(function<Expr(Expr)>, function<OutTy(InTy)>);

namespace {

template<typename T>
void assert_eq(T exp, T act) { ASSERT_EQ(exp, act); }

template<>
void assert_eq(float exp, float act) { ASSERT_FLOAT_EQ(exp, act); }

template<>
void assert_eq(double exp, double act) { ASSERT_DOUBLE_EQ(exp, act); }

}  // namespace

// Math ops tests
void iadd_test();
void fadd_test();
void isub_test();
void fsub_test();
void imul_test();
void fmul_test();
void idiv_test();
void udiv_test();
void fdiv_test();
void imod_test();
void imax_test();
void umax_test();
void fmax_test();
void imin_test();
void umin_test();
void fmin_test();
void ineg_test();
void fneg_test();
void dneg_test();
void fsqrt_test();
void dsqrt_test();
void fpow_test();
void dpow_test();
void fceil_test();
void dceil_test();
void ffloor_test();
void dfloor_test();
void fabs_test();
void dabs_test();
void iabs_test();

// Cast op test
void sitofp_test();
void uitofp_test();
void fptosi_test();
void fptoui_test();
void int8toint32_test();

#endif  // TEST_TEST_BASE_H_
