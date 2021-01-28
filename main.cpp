#include "till/core/expr.h"
#include "till/ext/exprs.h"
#include "till/ext/ops.h"
#include "till/ext/lstreams.h"

#include <iostream>
#include <memory>

using namespace std;
using namespace till;

int main()
{
    auto in = make_shared<FreeLine>("in");
    auto in_stream = make_shared<LStream<int>>(in);

    auto var = make_shared<Var<int>>();
    auto ten = make_shared<Const<int>>(10);
    auto add = make_shared<Add<int>>(var, ten);
    auto lambda = make_shared<Lambda<int, int>>(add, var);
    auto sel = make_shared<SelectOp<int, int>>("sel", lambda, in_stream);

    auto agg = make_shared<PeriodLine<0, 10>>("agg");
    auto aggexpr = make_shared<Agg<int, int>>();
    auto aggop = make_shared<AggOp<int, int>>(agg, aggexpr, sel);

    auto out = make_shared<PeriodLine<0, 100>>("out");
    auto lsop = make_shared<LStreamOp<int, int>>(out, aggop, in_stream);

    return 0;
}