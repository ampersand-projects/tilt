#include <iostream>
#include <memory>
#include <cstdlib>
#include <chrono>

#include "tilt/codegen/printer.h"
#include "tilt/codegen/loopgen.h"
#include "tilt/codegen/llvmgen.h"
#include "tilt/codegen/vinstr.h"
#include "tilt/engine/engine.h"
#include "tilt/builder/tilder.h"

using namespace std;
using namespace std::chrono;
using namespace tilt;
using namespace tilt::tilder;

Expr Count(Sym win)
{
    auto e = _elem(win, _pt(0));
    auto e_sym = e->sym("e");
    auto one = _f32(1);
    auto count_sel_sym = one->sym("count_sel");
    auto count_op = _op(
        _iter(0, 1),
        Params{ win },
        SymTable{ {e_sym, e}, {count_sel_sym, one} },
        _exists(e_sym),
        count_sel_sym);
    auto count_init = _f32(0);
    auto count_acc = [](Expr a, Expr b) { return _add(a, b); };
    auto count_expr = _agg(count_op, count_init, count_acc);
    return count_expr;
}

Expr Sum(Sym win)
{
    auto e = _elem(win, _pt(0));
    auto e_sym = e->sym("e");
    auto count_op = _op(
        _iter(0, 1),
        Params{ win },
        SymTable{ {e_sym, e} },
        _exists(e_sym),
        e_sym);
    auto count_init = _f32(0);
    auto count_acc = [](Expr a, Expr b) { return _add(a, b); };
    auto count_expr = _agg(count_op, count_init, count_acc);
    return count_expr;
}

Op WindowAvg(Sym in, int64_t w)
{
    auto window = _subls(in, _win(-w, 0));
    auto window_sym = window->sym("win");
    auto count = Count(window_sym);
    auto count_sym = count->sym("count");
    auto sum = Sum(window_sym);
    auto sum_sym = sum->sym("sum");
    auto avg = _div(sum_sym, count_sym);
    auto avg_sym = avg->sym("avg");
    auto wc_op = _op(
        _iter(0, w),
        Params{ in },
        SymTable{ {window_sym, window}, {count_sym, count}, {sum_sym, sum}, {avg_sym, avg} },
        _true(),
        avg_sym);
    return wc_op;
}

Op Join(Sym left, Sym right)
{
    auto e_left = _elem(left, _pt(0));
    auto e_left_sym = e_left->sym("left");
    auto e_right = _elem(right, _pt(0));
    auto e_right_sym = e_right->sym("right");
    auto norm = _sub(e_left_sym, e_right_sym);
    auto norm_sym = norm->sym("norm");
    auto left_exist = _exists(e_left_sym);
    auto right_exist = _exists(e_right_sym);
    auto join_cond = _and(left_exist, right_exist);
    auto join_op = _op(
        _iter(0, 1),
        Params{ left, right },
        SymTable{
            {e_left_sym, e_left},
            {e_right_sym, e_right},
            {norm_sym, norm},
        },
        join_cond,
        norm_sym);
    return join_op;
}

Op SelectSub(Sym in, Sym avg)
{
    auto e = _elem(in, _pt(0));
    auto e_sym = e->sym("e");
    auto res = _sub(e_sym, avg);
    auto res_sym = res->sym("res");
    auto sel_op = _op(
        _iter(0, 1),
        Params{in, avg},
        SymTable{{e_sym, e}, {res_sym, res}},
        _exists(e_sym),
        res_sym);
    return sel_op;
}

Op SelectDiv(Sym in, Sym std)
{
    auto e = _elem(in, _pt(0));
    auto e_sym = e->sym("e");
    auto res = _div(e_sym, std);
    auto res_sym = res->sym("res");
    auto sel_op = _op(
        _iter(0, 1),
        Params{in, std},
        SymTable{{e_sym, e}, {res_sym, res}},
        _exists(e_sym),
        res_sym);
    return sel_op;
}

Expr Average(Sym win)
{
    auto e = _elem(win, _pt(0));
    auto e_sym = e->sym("e");
    auto state = _new(vector<Expr>{e_sym, _f32(1)});
    auto state_sym = state->sym("state");
    auto count_op = _op(
        _iter(0, 1),
        Params{ win },
        SymTable{ {e_sym, e}, {state_sym, state} },
        _exists(e_sym),
        state_sym);
    auto count_init = _new(vector<Expr>{_f32(0), _f32(0)});
    auto count_acc = [](Expr a, Expr b) {
        auto sum = _get(a, 0);
        auto count = _get(a, 1);
        auto e = _get(b, 0);
        auto c = _get(b, 1);
        return _new(vector<Expr>{_add(sum, e), _add(count, c)});
    };
    auto count_expr = _agg(count_op, count_init, count_acc);
    return count_expr;
}

Expr StdDev(Sym win)
{
    auto e = _elem(win, _pt(0));
    auto e_sym = e->sym("e");
    auto state = _new(vector<Expr>{_mul(e_sym, e_sym), _f32(1)});
    auto state_sym = state->sym("state");
    auto count_op = _op(
        _iter(0, 1),
        Params{ win },
        SymTable{ {e_sym, e}, {state_sym, state} },
        _exists(e_sym),
        state_sym);
    auto count_init = _new(vector<Expr>{_f32(0), _f32(0)});
    auto count_acc = [](Expr a, Expr b) {
        auto sum = _get(a, 0);
        auto count = _get(a, 1);
        auto e = _get(b, 0);
        auto c = _get(b, 1);
        return _new(vector<Expr>{_add(sum, e), _add(count, c)});
    };
    auto count_expr = _agg(count_op, count_init, count_acc);
    return count_expr;
}

Op Norm(Sym in, int64_t len)
{
    auto inwin = _subls(in, _win(-len, 0));
    auto inwin_sym = inwin->sym("inwin");

    // avg state
    auto avg_state = Average(inwin_sym);
    auto avg_state_sym = avg_state->sym("avg_state");

    // avg value
    auto avg = _div(_get(avg_state_sym, 0), _get(avg_state_sym, 1));
    auto avg_sym = avg->sym("avg");

    // avg join
    auto avg_op = SelectSub(inwin_sym, avg_sym);
    auto avg_op_sym = avg_op->sym("avgop");

    // stddev state
    auto std_state = StdDev(avg_op_sym);
    auto std_state_sym = std_state->sym("stddev_state");

    // stddev value
    auto std = _sqrt(_div(_get(std_state_sym, 0), _get(std_state_sym, 1)));
    auto std_sym = std->sym("std");

    // std join
    auto std_op = SelectDiv(avg_op_sym, std_sym);
    auto std_op_sym = std_op->sym("stdop");

    // query operation
    auto query_op = _op(
        _iter(0, len),
        Params{ in },
        SymTable{
            {inwin_sym, inwin},
            {avg_state_sym, avg_state},
            {avg_sym, avg},
            {avg_op_sym, avg_op},
            {std_state_sym, std_state},
            {std_sym, std},
            {std_op_sym, std_op}
        },
        _true(),
        std_op_sym);

    return query_op;
}

Op MovingSum(Sym in)
{
    auto e = _elem(in, _pt(0));
    auto e_sym = e->sym("e");
    auto p = _elem(in, _pt(-3));
    auto p_sym = p->sym("p");
    auto o = _elem(_out(tilt::Type(types::INT32, _iter(0, -1))), _pt(-1));
    auto o_sym = o->sym("o");
    auto p_val = _ifelse(_exists(p_sym), p_sym, _i32(0));
    auto p_val_sym = p_val->sym("p_val");
    auto o_val = _ifelse(_exists(o_sym), o_sym, _i32(0));
    auto o_val_sym = o_val->sym("o_val");
    auto res = _sub(_add(e_sym, o_val_sym), p_val_sym);
    auto res_sym = res->sym("res");
    auto sel_op = _op(
        _iter(0, 1),
        Params{in},
        SymTable{
            {e_sym, e},
            {p_sym, p},
            {o_sym, o},
            {p_val_sym, p_val},
            {o_val_sym, o_val},
            {res_sym, res},
        },
        _exists(e_sym),
        res_sym);
    return sel_op;
}

int main(int argc, char** argv)
{
    int dlen = (argc > 1) ? atoi(argv[1]) : 30;
    int len = (argc > 2) ? atoi(argv[2]) : 10;

    // input stream
    auto in_sym = _sym("in", tilt::Type(types::INT32, _iter("in")));

    auto query_op = MovingSum(in_sym);
    auto query_op_sym = query_op->sym("query");
    cout << endl << "TiLT IR:" << endl;
    cout << IRPrinter::Build(query_op) << endl;

    auto loop = LoopGen::Build(query_op_sym, query_op.get());
    cout << endl << "Loop IR:" << endl;
    cout << IRPrinter::Build(loop);

    auto jit = ExecEngine::Get();
    auto& llctx = jit->GetCtx();
    auto llmod = LLVMGen::Build(loop, llctx);
    cout << endl << "LLVM IR:" << endl;
    cout << IRPrinter::Build(llmod.get()) << endl;

    jit->AddModule(move(llmod));

    auto loop_addr = (region_t* (*)(ts_t, ts_t, region_t*, region_t*)) jit->Lookup(loop->get_name());

    uint32_t dur = 1;

    auto in_tl = new ival_t[dlen];
    auto in_data = new int[dlen];
    region_t in_reg;
    init_region(&in_reg, 0, get_buf_size(dlen), in_tl, reinterpret_cast<char*>(in_data));
    for (int i = 0; i < dlen; i++) {
        auto t = dur*i;
        in_reg.et = t;
        in_reg.ei++;
        in_tl[in_reg.ei] = {t, dur};
        in_data[in_reg.ei] = i%1000 + 5;
    }

    auto out_tl = new ival_t[dlen];
    auto out_data = new int[dlen];
    region_t out_reg;
    init_region(&out_reg, 0, get_buf_size(dlen), out_tl, reinterpret_cast<char*>(out_data));

    cout << "Query execution: " << endl;
    auto start_time = high_resolution_clock::now();
    auto* res_reg = loop_addr(0, dur*dlen, &out_reg, &in_reg);
    auto end_time = high_resolution_clock::now();

    int out_count = dlen;
    if (argc == 1) {
        for (int i = 0; i < dlen; i++) {
            cout << "(" << in_tl[i].t << "," << in_tl[i].d << ") " << in_data[i] << " -> "
                << "(" << out_tl[i].t << "," << out_tl[i].d << ") " << out_data[i] << endl;
        }
    }

    auto time = duration_cast<microseconds>(end_time - start_time).count();
    cout << "Data size: " << out_count << " Time: " << time << endl;

    return 0;
}
