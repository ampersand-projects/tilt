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

#endif  // TEST_TEST_BASE_H_
