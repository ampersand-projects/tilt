#ifndef TILT_TEST_BASE
#define TILT_TEST_BASE

#include "tilt/ir/op.h"

#include <functional>

using namespace std;
using namespace tilt;

template<typename T>
struct Event {
    long st;
    long et;
    T payload;
};

template<typename InTy, typename OutTy>
using QueryFn = function<vector<Event<OutTy>>(vector<Event<InTy>>)>;

template<typename InTy, typename OutTy>
void op_test(Op, QueryFn<InTy, OutTy>, vector<Event<InTy>>);

template<typename InTy, typename OutTy>
void select_test(function<Expr(Expr)>, function<OutTy(InTy)>);

// Math op tests
void addop_test();

#endif