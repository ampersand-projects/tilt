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
using namespace tilt::tilder;

Op Select(Sym in)
{
    auto e = _elem(in, _pt(0));
    auto e_sym = e->sym("e");
    auto ten = _i32(10);
    auto add = _add(_get(e_sym, 0), ten);
    auto elem_exists = _exists(e_sym);
    auto sel_sym = add->sym("selector");
    auto sel_op = _op(
        _iter(0, 1),
        Params{ in },
        SymTable{ {e_sym, e}, {sel_sym, add} },
        elem_exists,
        sel_sym);
    return sel_op;
}

Expr Count(Sym win)
{
    auto cur = _elem(win, _pt(0));
    auto cur_sym = cur->sym("cur");
    auto cur_exists = _exists(cur_sym);
    auto one = _i32(1);
    auto count_sel_sym = one->sym("count_sel");
    auto count_op = _op(
        _iter(0, 1),
        Params{ win },
        SymTable{ {cur_sym, cur}, {count_sel_sym, one} },
        cur_exists,
        count_sel_sym);
    auto count_init = _i32(0);
    auto count_acc = [](Expr a, Expr b) { return _add(a, b); };
    auto count_expr = _agg(count_op, count_init, count_acc);
    return count_expr;
}

Op WindowCount(Sym in, long w)
{
    auto window = _subls(in, _win(-w, 0));
    auto window_sym = window->sym("win");
    auto count = Count(window_sym);
    auto count_sym = count->sym("count");
    auto wc_op = _op(
        _iter(0, w),
        Params{ in },
        SymTable{ {window_sym, window}, {count_sym, count} },
        _true(),
        count_sym);
    return wc_op;
}

Op Join(Sym left, Sym right)
{
    auto e_left = _elem(left, _pt(0));
    auto e_left_sym = e_left->sym("left");
    auto e_right = _elem(right, _pt(0));
    auto e_right_sym = e_right->sym("right");
    auto accum = _new(vector<Expr>{e_left_sym, e_right_sym});
    auto accum_sym = accum->sym("accum");
    auto left_exist = _exists(e_left_sym);
    auto right_exist = _exists(e_right_sym);
    auto join_cond = _and(left_exist, right_exist);
    auto join_op = _op(
        _iter(0, 1),
        Params{ left, right },
        SymTable{ {e_left_sym, e_left}, {e_right_sym, e_right}, {accum_sym, accum} },
        join_cond,
        accum_sym);
    return join_op;
}

Op Query(Sym in, long len, long w)
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
    auto query_op = _op(
        _iter(0, len),
        Params{ in },
        SymTable{ {inwin_sym, inwin}, {sel_op_sym, sel_op}, {wc_op_sym, wc_op}, {join_op_sym, join_op} },
        _true(),
        join_op_sym);

    return query_op;
}

int main(int argc, char** argv)
{
    // input stream
    auto in_sym = _sym("in", tilt::Type(types::STRUCT<int32_t, int32_t>(), _iter("in")));

    auto query_op = Query(in_sym, 10, 10);
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
    auto init_region = (region_t* (*)(region_t*, long, index_t*, char*)) jit->Lookup("init_region");

    int dlen = (argc>1) ? atoi(argv[1]) : 30;
    unsigned int dur = 5;

    struct Data {
        int a;
        int b;
    };

    auto in_tl = new index_t[dlen];
    auto in_data = new Data[dlen];
    region_t in_reg;
    init_region(&in_reg, 0, in_tl, (char*)in_data);
    for (int i=0; i<dlen; i++) {
        auto t = dur*i;
        in_reg.ei.t = t;
        in_reg.ei.i++;
        in_tl[in_reg.ei.i] = {t, dur};
        in_data[in_reg.ei.i] = {i, 3};
    }

    auto out_tl = new index_t[dlen];
    auto out_data = new Data[dlen];
    region_t out_reg;
    init_region(&out_reg, 0, out_tl, (char*)out_data);

    auto start_time = high_resolution_clock::now();
    auto* res_reg = loop_addr(0, dur*dlen, &out_reg, &in_reg);
    auto end_time = high_resolution_clock::now();

    int out_count = 0;
    for (int i=0; i<dlen; i++) {
        if (argc == 1) {
            cout << "(" << in_tl[i].t << "," << in_tl[i].i << ") " << in_data[i].a << "," << in_data[i].b << " -> "
                << "(" << out_tl[i].t << "," << out_tl[i].i << ") " << out_data[i].a << "," << out_data[i].b << endl;
        }
        out_count += ((out_data[i].a == in_data[i].a+10) && (out_data[i].b == 2));
    }

    auto time = duration_cast<microseconds>(end_time - start_time).count();
    cout << "Data size: " << out_count << " Time: " << time << endl;

    return 0;
}
