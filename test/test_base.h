#ifndef TEST_TEST_BASE_H_
#define TEST_TEST_BASE_H_

#include <functional>
#include <vector>

#include "tilt/ir/op.h"

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

// Math op tests
void addop_test();

#endif  // TEST_TEST_BASE_H_
