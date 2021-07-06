#include "tilt/base/type.h"
#include "tilt/ir/expr.h"
#include "tilt/ir/lstream.h"
#include "tilt/ir/op.h"
#include "tilt/codegen/printer.h"
#include "tilt/codegen/loopgen.h"
#include "tilt/codegen/llvmgen.h"
#include "tilt/engine/engine.h"

#include <iostream>
#include <memory>
#include <cstdlib>
#include <chrono>

using namespace std;
using namespace std::chrono;
using namespace tilt;

OpPtr Select(SymPtr in)
{
    auto elem = make_shared<Element>(in, Point());
    auto elem_sym = elem->GetSym("e");
    auto ten = make_shared<IConst>(types::INT32, 10);
    auto add = make_shared<Add>(elem_sym, ten);
    auto elem_exists = make_shared<Exists>(elem_sym);
    auto sel_sym = add->GetSym("selector");
    auto sel_op = make_shared<Op>(
        in->type.tl,
        FreqIter(0, 1),
        Params{ in },
        elem_exists,
        SymTable{ {elem_sym, elem}, { sel_sym, add } },
        sel_sym);
    return sel_op;
}

OpPtr NestedSelect(SymPtr in, long w)
{
    auto inwin = make_shared<SubLStream>(in, Window(-w, 0));
    auto inwin_sym = inwin->GetSym("inwin");
    auto sel = Select(inwin_sym);
    auto sel_sym = sel->GetSym("sel");
    auto sel_op = make_shared<Op>(
        in->type.tl,
        FreqIter(0, w),
        Params{ in },
        make_shared<True>(),
        SymTable{ {inwin_sym, inwin}, { sel_sym, sel } },
        sel_sym);
    return sel_op;
}

ExprPtr Count(SymPtr win)
{
    auto cur = make_shared<Element>(win, Point());
    auto cur_sym = cur->GetSym("cur");
    auto cur_exists = make_shared<Exists>(cur_sym);
    auto one = make_shared<IConst>(types::INT32, 1);
    auto count_sel_sym = one->GetSym("count_sel");
    auto count_op = make_shared<Op>(
        win->type.tl,
        FreqIter(0, 1),
        Params{ win },
        cur_exists,
        SymTable{ {cur_sym, cur}, { count_sel_sym, one } },
        count_sel_sym);
    auto count_expr = make_shared<Sum>(count_op);
    return count_expr;
}

OpPtr WindowCount(SymPtr in, long w)
{
    auto win = make_shared<SubLStream>(in, Window(-w, 0));
    auto win_sym = win->GetSym("win");
    auto count = Count(win_sym);
    auto count_sym = count->GetSym("count");
    auto wc_op = make_shared<Op>(
        Timeline(FreqIter(0, w)),
        FreqIter(0, w),
        Params{ in },
        make_shared<True>(),
        SymTable{ {win_sym, win}, { count_sym, count } },
        count_sym);
    return wc_op;
}

OpPtr Join(SymPtr left, SymPtr right)
{
    auto e_left = make_shared<Element>(left, Point());
    auto e_left_sym = e_left->GetSym("left");
    auto e_right = make_shared<Element>(right, Point());
    auto e_right_sym = e_right->GetSym("right");
    auto accum = make_shared<Add>(e_left_sym, e_right_sym);
    auto accum_sym = accum->GetSym("accum");
    auto left_exist = make_shared<Exists>(e_left_sym);
    auto right_exist = make_shared<Exists>(e_right_sym);
    auto join_cond = make_shared<And>(left_exist, right_exist);
    auto join_op = make_shared<Op>(
        Timeline{left->type.tl.iters[0], right->type.tl.iters[0]},
        FreqIter(0, 1),
        Params{ left, right },
        join_cond,
        SymTable{ {e_left_sym, e_left}, {e_right_sym, e_right}, {accum_sym, accum} },
        accum_sym);
    return join_op;
}

OpPtr Query(SymPtr in, long len, long w)
{
    auto inwin = make_shared<SubLStream>(in, Window(-len, 0));
    auto inwin_sym = inwin->GetSym("inwin");

    // select operation
    auto sel_op = Select(inwin_sym);
    auto sel_op_sym = sel_op->GetSym("sel");

    // count aggregate operation
    auto wc_op = WindowCount(inwin_sym, w);
    auto wc_op_sym = wc_op->GetSym("wc");

    // join operation
    auto join_op = Join(sel_op_sym, wc_op_sym);
    auto join_op_sym = join_op->GetSym("join");

    // query operation
    auto query_op = make_shared<Op>(
        join_op_sym->type.tl,
        FreqIter(0, len),
        Params{ in },
        make_shared<True>(),
        SymTable{ {inwin_sym, inwin}, {sel_op_sym, sel_op}, {wc_op_sym, wc_op}, {join_op_sym, join_op} },
        join_op_sym);

    return query_op;
}

int main(int argc, char** argv)
{
    // input stream
    auto in_sym = make_shared<Symbol>("in", tilt::Type(types::INT32, FreeIter("in")));

    auto query_op = NestedSelect(in_sym, 5);
    auto query_op_sym = query_op->GetSym("query");

    cout << endl << "TiLT IR: " << endl;
    IRPrinter printer;
    query_op->Accept(printer);
    cout << printer.result() << endl;

    cout << endl << "Loop IR: " << endl;
    IRPrinter loop_printer;
    auto loop = LoopGen::Build(query_op_sym, query_op.get());
    loop->Accept(loop_printer);
    cout << loop_printer.result() << endl;

    auto jit = ExecEngine::Get();

    cout << endl << "LLVM IR: " << endl;
    auto& llctx = jit->GetCtx();
    llvm::IRBuilder<> builder(llctx);
    auto llmod = LLVMGen::Build(loop, llctx, builder);
    llvm::raw_fd_ostream r(fileno(stdout), false);
    if (!llvm::verifyModule(*llmod, &r)) { r << *llmod; }

    cout << endl << endl << "Query execution: " << endl;

    jit->AddModule(move(llmod));

    auto loop_addr = (region_t* (*)(long, long, region_t*, region_t*)) jit->Lookup(loop->GetName());

    int dlen = (argc>1) ? atoi(argv[1]) : 31;

    auto in_tl = new index_t[dlen];
    auto in_data = new int[dlen];
    region_t in_reg;
    in_reg.si.i = 0;
    in_reg.si.t = 0;
    in_reg.ei.i = dlen-1;
    in_reg.ei.t = dlen-1;
    in_reg.tl = in_tl;
    in_reg.data = (char*) in_data;

    for (int i=0; i<dlen; i++) {
        in_tl[i] = {i, 1};
        in_data[i] = i;
    }

    auto out_tl = new index_t[dlen];
    auto out_data = new int[dlen];
    region_t out_reg;
    out_reg.si.i = 0;
    out_reg.si.t = 0;
    out_reg.ei.i = 0;
    out_reg.ei.t = 0;
    out_reg.tl = out_tl;
    out_reg.data = (char*) out_data;

    auto start_time = high_resolution_clock::now();
    auto* res_reg = loop_addr(0, dlen-1, &out_reg, &in_reg);
    auto end_time = high_resolution_clock::now();

    int out_count = 0;
    for (int i=1; i<dlen; i++) {
        if (argc == 1) {
            cout << "(" << in_tl[i].t << "," << in_tl[i].i << ") " << in_data[i] << " -> "
                << "(" << out_tl[i].t << "," << out_tl[i].i << ") " << out_data[i] << endl;
        }
        out_count += (out_data[i] == in_data[i]+10);
    }

    auto dur = duration_cast<microseconds>(end_time - start_time).count();
    cout << "Data size: " << out_count << " Time: " << dur << endl;

    return 0;
}
