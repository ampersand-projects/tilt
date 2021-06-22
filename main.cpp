#include "tilt/base/type.h"
#include "tilt/ir/expr.h"
#include "tilt/ir/lstream.h"
#include "tilt/ir/op.h"
#include "tilt/codegen/printer.h"
#include "tilt/codegen/loopgen.h"
#include "tilt/codegen/llvmgen.h"

#include <iostream>
#include <memory>

using namespace std;
using namespace tilt;

int main()
{
    // input stream
    auto in_sym = make_shared<Symbol>("in", tilt::Type(types::INT32, FreeIter("in")));

    unsigned long len = 500;
    auto inwin = make_shared<SubLStream>(in_sym, Window(-len, 0));
    auto inwin_sym = inwin->GetSym("inwin");

    // select operation
    auto elem = make_shared<Element>(inwin_sym, Point());
    auto elem_sym = elem->GetSym("e");
    auto ten = make_shared<IConst>(types::INT32, 10);
    auto add = make_shared<Add>(elem_sym, ten);
    auto elem_exists = make_shared<Exists>(elem_sym);
    auto sel_sym = add->GetSym("selector");
    auto sel_op = make_shared<Op>(
        inwin_sym->type.tl,
        FreqIter(0, 1),
        Params{ inwin_sym },
        elem_exists,
        SymTable{ {elem_sym, elem}, { sel_sym, add } },
        sel_sym);
    auto sel_op_sym = sel_op->GetSym("sel");

    // count aggregate operation
    unsigned long w = 100;
    auto win = make_shared<SubLStream>(inwin_sym, Window(-w, 0));
    auto win_sym = win->GetSym("win");

    // sum expr for counting
    auto cur = make_shared<Element>(win_sym, Point());
    auto cur_sym = cur->GetSym("cur");
    auto cur_exists = make_shared<Exists>(cur_sym);
    auto one = make_shared<IConst>(types::INT32, 1);
    auto count_sel_sym = one->GetSym("count_sel");
    auto count_op = make_shared<Op>(
        win_sym->type.tl,
        FreqIter(0, 1),
        Params{ win_sym },
        cur_exists,
        SymTable{ {cur_sym, cur}, { count_sel_sym, one } },
        count_sel_sym);
    auto count = make_shared<Sum>(count_op);
    auto count_sym = count->GetSym("count");
    auto wc_op = make_shared<Op>(
        Timeline(FreqIter(0, w)),
        FreqIter(0, w),
        Params{ inwin_sym },
        make_shared<True>(),
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
    auto left_exist = make_shared<Exists>(left_sym);
    auto right_exist = make_shared<Exists>(right_sym);
    auto join_cond = make_shared<And>(left_exist, right_exist);
    auto join_op = make_shared<Op>(
        Timeline{sel_op_sym->type.tl.iters[0], wc_op_sym->type.tl.iters[0]},
        FreqIter(0, 1),
        Params{ sel_op_sym, wc_op_sym },
        join_cond,
        SymTable{ {left_sym, left}, {right_sym, right}, {accum_sym, accum} },
        accum_sym);
    auto join_op_sym = join_op->GetSym("join");

    // query operation
    auto query_op = make_shared<Op>(
        join_op_sym->type.tl,
        FreqIter(0, len),
        Params{ in_sym },
        make_shared<True>(),
        SymTable{ {inwin_sym, inwin}, {sel_op_sym, sel_op}, {wc_op_sym, wc_op}, {join_op_sym, join_op} },
        join_op_sym);

    auto nest_sel_op = make_shared<Op>(
        Timeline{sel_op_sym->type.tl.iters[0], FreqIter(0, w)},
        FreqIter(0, len),
        Params{in_sym},
        make_shared<True>(),
        SymTable{ {inwin_sym, inwin}, {sel_op_sym, sel_op} },
        sel_op_sym
    );
    auto nest_sel_op_sym = nest_sel_op->GetSym("nest_sel");

    cout << endl << "TiLT IR: " << endl;
    IRPrinter printer;
    sel_op->Accept(printer);
    cout << printer.result() << endl;

    cout << endl << "Loop IR: " << endl;
    IRPrinter loop_printer;
    auto loop = LoopGen::Build(sel_op_sym, sel_op.get());
    loop->Accept(loop_printer);
    cout << loop_printer.result() << endl;

    cout << endl << "LLVM IR: " << endl;
    auto llctx = LLVMGen::Build(loop);
    auto llmod = llctx.llmod();
    llvm::raw_fd_ostream r(fileno(stdout), false);
    if (!llvm::verifyModule(*llmod, &r)) { r << *llmod; }

    return 0;
}