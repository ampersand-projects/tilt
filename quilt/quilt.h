#ifndef QUILT_QUILT_H_
#define QUILT_QUILT_H_

#include "tilt/builder/tilder.h"

using namespace tilt;
using namespace tilt::tilder;

Op _Select(Sym, function<Expr(Expr)>);
Op SelectSub(Sym, Sym);
Op SelectDiv(Sym, Sym);
Op MovingSum(Sym, int64_t, int64_t);
Op WindowAvg(Sym, int64_t);
Op Join(Sym, Sym);
Op Norm(Sym, int64_t);

Expr Count(Sym);
Expr Sum(Sym);
Expr Average(Sym);
Expr StdDev(Sym);

#endif  // QUILT_QUILT_H_
