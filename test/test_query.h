#ifndef TEST_TEST_QUERY_H_
#define TEST_TEST_QUERY_H_

#include "tilt/builder/tilder.h"

using namespace tilt;

Op _Select(Sym, function<Expr(Expr)>);
Op _MovingSum(Sym, int64_t, int64_t);

#endif  // TEST_TEST_QUERY_H_
