#include "till/base/type.h"
#include "till/ir/expr.h"
#include "till/ir/lstream.h"
#include "till/ir/op.h"
#include "till/codegen/printer.h"

#include <iostream>
#include <memory>

using namespace std;
using namespace till;

int main()
{
    // input stream
    auto in_sym = make_shared<Symbol>("in", Type(types::INT32, Iter(0, 1)));

    // select operation
    auto elem = make_shared<Element>(in_sym, Point());
    auto elem_sym = make_shared<Symbol>("e", elem->type);
    auto ten = make_shared<IConst>(types::INT32, 10);
    auto add = make_shared<Add>(elem_sym, ten);
    auto sel = make_shared<Lambda>(vector<SymPtr>{elem_sym}, add);
    auto sel_sym = make_shared<Symbol>("selector", sel->type);
    auto sel_op = make_shared<Op>(
        Iter(0, 1),
        Params{ in_sym },
        SymTable{ {elem_sym, elem}, { sel_sym, sel } },
        sel_sym);
    auto sel_op_sym = make_shared<Symbol>("sel", sel_op->type);

    // count aggregate operation
    ulong w = 100;
    auto win = make_shared<SubLStream>(in_sym, Window(0, w));
    auto win_sym = make_shared<Symbol>("win", win->type);
    // sum expr for counting
    auto win_elem = make_shared<Element>(win_sym, Point());
    auto win_elem_sym = make_shared<Symbol>("we", win_elem->type);
    auto one = make_shared<IConst>(types::INT32, 1);
    auto count_sel = make_shared<Lambda>(vector<SymPtr>{win_elem_sym}, one);
    auto count_sel_sym = make_shared<Symbol>("count_sel", count_sel->type);
    auto count_op = make_shared<Op>(
        Iter(0, 1),
        Params{ win_sym },
        SymTable{ {win_elem_sym, win_elem}, { count_sel_sym, count_sel } },
        count_sel_sym);
    auto count = make_shared<Sum>(count_op);
    auto count_sym = make_shared<Symbol>("count", count->type);
    auto wc_op = make_shared<Op>(
        Iter(0, w),
        Params{ in_sym },
        SymTable{ {win_sym, win}, { count_sym, count } },
        count_sym);
    auto wc_op_sym = make_shared<Symbol>("wc", wc_op->type);

    // join operation
    auto left = make_shared<Element>(sel_op_sym, Point());
    auto left_sym = make_shared<Symbol>("left", left->type);
    auto right = make_shared<Element>(wc_op_sym, Point());
    auto right_sym = make_shared<Symbol>("right", right->type);
    auto accum = make_shared<Add>(left_sym, right_sym);
    auto accum_sym = make_shared<Symbol>("accum", accum->type);
    auto join_op = make_shared<Op>(
        Iter(0, 1),
        Params{ sel_op_sym, wc_op_sym },
        SymTable{ {left_sym, left}, {right_sym, right}, {accum_sym, accum} },
        accum_sym);
    auto join_op_sym = make_shared<Symbol>("join", join_op->type);

    // query operation
    auto query_op = make_shared<Op>(
        OnceIter(),
        Params{ in_sym },
        SymTable{ {sel_op_sym, sel_op}, {wc_op_sym, wc_op}, {join_op_sym, join_op} },
        join_op_sym);

    IRPrinter printer;
    printer.Visit(*query_op);
    cout << printer.str() << endl;

    return 0;
}