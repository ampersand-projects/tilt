#ifndef QUILT_INCLUDE_QUILT_QUILT_H_
#define QUILT_INCLUDE_QUILT_QUILT_H_

#include "tilt/builder/tilder.h"

using namespace tilt;
using namespace tilt::tilder;

Op _Select(Sym, function<Expr(Expr)>);
Op _MovingSum(Sym, int64_t, int64_t);
Op _WindowAvg(Sym, int64_t);
Op _Join(Sym, Sym);
Op _Norm(Sym, int64_t);

Expr _Count(Sym);
Expr _Sum(Sym);
Expr _Average(Sym);
Expr _StdDev(Sym);

#endif  // QUILT_INCLUDE_QUILT_QUILT_H_
