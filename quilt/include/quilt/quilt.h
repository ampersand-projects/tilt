#ifndef QUILT_INCLUDE_QUILT_QUILT_H_
#define QUILT_INCLUDE_QUILT_QUILT_H_

#include "tilt/builder/tilder.h"

using namespace std;
using namespace tilt;
using namespace tilt::tilder;

Op _Select(_sym, function<Expr(Expr)>);
Op _MovingSum(_sym, int64_t, int64_t);
Op _Join(_sym, _sym);
Op _WindowAvg(string, _sym, int64_t);
Op _Norm(string, _sym, int64_t);
Op _Resample(string, _sym, int64_t, int64_t);

Expr _Count(_sym);
Expr _Sum(_sym);
Expr _Average(_sym);
Expr _StdDev(_sym);

#endif  // QUILT_INCLUDE_QUILT_QUILT_H_
