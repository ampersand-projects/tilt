#include "tilt/base/type.h"
#include "tilt/ir/expr.h"
#include "tilt/ir/lstream.h"
#include "tilt/ir/op.h"
#include "tilt/codegen/printer.h"

#include <iostream>
#include <memory>

using namespace std;
using namespace tilt;

int main()
{
    // input stream
    auto in_sym = make_shared<Symbol>("in", Type(types::INT32, FreeIter()));

    // select operation
    auto elem = make_shared<Element>(in_sym, Point());
    auto elem_sym = elem->GetSym("e");
    auto ten = make_shared<IConst>(types::INT32, 10);
    auto add = make_shared<Add>(elem_sym, ten);
    auto exists = make_shared<Exists>(elem_sym);
    auto sel = make_shared<Lambda>(vector<SymPtr>{elem_sym}, exists, add);
    auto sel_sym = sel->GetSym("selector");
    auto sel_op = make_shared<Op>(
        in_sym->type.tl,
        Iterator(0, 1),
        Params{ in_sym },
        SymTable{ {elem_sym, elem}, { sel_sym, sel } },
        sel_sym);
    auto sel_op_sym = sel_op->GetSym("sel");

    // count aggregate operation
    unsigned long w = 100;
    auto win = make_shared<SubLStream>(in_sym, Window(-w, 0));
    auto win_sym = win->GetSym("win");

    // sum expr for counting
    auto cur = make_shared<Element>(win_sym, Point());
    auto cur_sym = cur->GetSym("cur");
    auto cur_exists = make_shared<Exists>(cur_sym);
    auto one = make_shared<IConst>(types::INT32, 1);
    auto count_sel = make_shared<Lambda>(vector<SymPtr>{cur_sym}, cur_exists, one);
    auto count_sel_sym = count_sel->GetSym("count_sel");
    auto count_op = make_shared<Op>(
        win_sym->type.tl,
        Iterator(0, 1),
        Params{ win_sym },
        SymTable{ {cur_sym, cur}, { count_sel_sym, count_sel } },
        count_sel_sym);
    auto count = make_shared<Sum>(count_op);
    auto count_sym = count->GetSym("count");
    auto wc_op = make_shared<Op>(
        Timeline(Iterator(0, w)),
        Iterator(0, w),
        Params{ in_sym },
        SymTable{ {win_sym, win}, { count_sym, count } },
        count_sym);
    auto wc_op_sym = wc_op->GetSym("wc");

    // join operation
    auto left = make_shared<Element>(sel_op_sym, Point());
    auto left_sym = left->GetSym("left");
    auto right = make_shared<Element>(wc_op_sym, Point());
    auto right_sym = right->GetSym("right");
    auto accum = make_shared<Add>(left_sym, right_sym);
    auto accum_sym = accum->GetSym("accum");
    auto join_op = make_shared<Op>(
        Timeline{sel_op_sym->type.tl.iters[0], wc_op_sym->type.tl.iters[0]},
        Iterator(0, 1),
        Params{ sel_op_sym, wc_op_sym },
        SymTable{ {left_sym, left}, {right_sym, right}, {accum_sym, accum} },
        accum_sym);
    auto join_op_sym = join_op->GetSym("join");

    // query operation
    auto query_op = make_shared<Op>(
        join_op_sym->type.tl,
        OnceIter(),
        Params{ in_sym },
        SymTable{ {sel_op_sym, sel_op}, {wc_op_sym, wc_op}, {join_op_sym, join_op} },
        join_op_sym);

    IRPrinter printer;
    printer.Visit(*query_op);
    cout << printer.str() << endl;

    return 0;
}