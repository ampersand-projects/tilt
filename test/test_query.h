#ifndef TEST_TEST_QUERY_H_
#define TEST_TEST_QUERY_H_

#include "tilt/builder/tilder.h"

using namespace tilt;
using namespace tilt::tilder;

Op _Select(_sym, function<Expr(Expr)>);
Op _MovingSum(_sym, int64_t, int64_t);

#endif  // TEST_TEST_QUERY_H_
