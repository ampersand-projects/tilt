#include "tilt/base/type.h"
#include "tilt/ir/expr.h"
#include "tilt/ir/lstream.h"
#include "tilt/ir/op.h"
#include "tilt/codegen/printer.h"
#include "tilt/codegen/loopgen.h"
#include "tilt/codegen/llvmgen.h"
#include "tilt/engine/engine.h"
#include "tilt/builder/tilder.h"

#include <iostream>
#include <memory>
#include <cstdlib>
#include <chrono>

using namespace std;
using namespace std::chrono;
using namespace tilt;

OpPtr Select(SymPtr in)
{
    auto elem = _elem(in, _pt(0));
    auto elem_sym = elem->sym("e");
    auto ten = _i32(10);
    auto add = _add(elem_sym, ten);
    auto elem_exists = _exists(elem_sym);
    auto sel_sym = add->sym("selector");
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
    auto inwin = _subls(in, _win(-w, 0));
    auto inwin_sym = inwin->sym("inwin");
    auto sel = Select(inwin_sym);
    auto sel_sym = sel->sym("sel");
    auto sel2 = Select(sel_sym);
    auto sel2_sym = sel2->sym("sel2");
    auto sel_op = make_shared<Op>(
        in->type.tl,
        FreqIter(0, w),
        Params{ in },
        make_shared<True>(),
        SymTable{ {inwin_sym, inwin}, {sel_sym, sel}, {sel2_sym, sel2} },
        sel2_sym);
    return sel_op;
}

ExprPtr Count(SymPtr win)
{
    auto cur = _elem(win, _pt(0));
    auto cur_sym = cur->sym("cur");
    auto cur_exists = _exists(cur_sym);
    auto one = _i32(1);
    auto count_sel_sym = one->sym("count_sel");
    auto count_op = make_shared<Op>(
        win->type.tl,
        FreqIter(0, 1),
        Params{ win },
        cur_exists,
        SymTable{ {cur_sym, cur}, { count_sel_sym, one } },
        count_sel_sym);
    auto count_init = _i32(0);
    auto count_acc = [](ExprPtr a, ExprPtr b) { return _add(a, b); };
    auto count_expr = make_shared<AggExpr>(count_op, count_init, count_acc);
    return count_expr;
}

OpPtr WindowCount(SymPtr in, long w)
{
    auto win = _subls(in, _win(-w, 0));
    auto win_sym = win->sym("win");
    auto count = Count(win_sym);
    auto count_sym = count->sym("count");
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
    auto e_left = _elem(left, _pt(0));
    auto e_left_sym = e_left->sym("left");
    auto e_right = _elem(right, _pt(0));
    auto e_right_sym = e_right->sym("right");
    auto accum = _add(e_left_sym, e_right_sym);
    auto accum_sym = accum->sym("accum");
    auto left_exist = _exists(e_left_sym);
    auto right_exist = _exists(e_right_sym);
    auto join_cond = _and(left_exist, right_exist);
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
    auto inwin = _subls(in, _win(-len, 0));
    auto inwin_sym = inwin->sym("inwin");

    // select operation
    auto sel_op = Select(inwin_sym);
    auto sel_op_sym = sel_op->sym("sel");

    // count aggregate operation
    auto wc_op = WindowCount(inwin_sym, w);
    auto wc_op_sym = wc_op->sym("wc");

    // join operation
    auto join_op = Join(sel_op_sym, wc_op_sym);
    auto join_op_sym = join_op->sym("join");

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
    auto in_sym = _ls("in", types::INT32);

    auto query_op = Query(in_sym, 10, 5);
    auto query_op_sym = query_op->sym("query");

    cout << endl << "TiLT IR: " << endl;
    cout << IRPrinter::Build(query_op) << endl;

    cout << endl << "Loop IR: " << endl;
    auto loop = LoopGen::Build(query_op_sym, query_op.get());
    cout << IRPrinter::Build(loop) << endl;

    auto jit = ExecEngine::Get();

    cout << endl << "LLVM IR: " << endl;
    auto& llctx = jit->GetCtx();
    auto llmod = LLVMGen::Build(loop, llctx);
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
    in_tl[0] = STARTER_CKPT;
    for (int i=1; i<dlen; i++) {
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
    out_tl[0] = STARTER_CKPT;

    auto start_time = high_resolution_clock::now();
    auto* res_reg = loop_addr(0, dlen-1, &out_reg, &in_reg);
    auto end_time = high_resolution_clock::now();

    int out_count = 0;
    for (int i=1; i<dlen; i++) {
        if (argc == 1) {
            cout << "(" << in_tl[i].t << "," << in_tl[i].i << ") " << in_data[i] << " -> "
                << "(" << out_tl[i].t << "," << out_tl[i].i << ") " << out_data[i] << endl;
        }
        out_count += (out_data[i] == in_data[i]+15);
    }

    auto dur = duration_cast<microseconds>(end_time - start_time).count();
    cout << "Data size: " << out_count << " Time: " << dur << endl;

    return 0;
}
