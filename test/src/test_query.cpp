#include <memory>
#include <cstdlib>
#include <vector>
#include <numeric>

#include "test_query.h"

Op _Select(_sym in, function<Expr(Expr)> sel_expr)
{
    auto e = in[_pt(0)];
    auto e_sym = _sym("e", e);
    auto sel = sel_expr(_get(e_sym, 0));
    auto sel_sym = _sym("sel", sel);
    auto sel_op = _op(
        in->type.iter,
        Params{ in },
        SymTable{ {e_sym, e}, {sel_sym, sel} },
        _exists(e_sym),
        sel_sym);
    return sel_op;
}

Expr _Count(_sym win)
{
    auto acc = [](Expr s, Expr st, Expr et, Expr d) { return _add(s, _f32(1)); };
    return _red(win, _f32(0), acc);
}

Expr _Sum(_sym win)
{
    auto acc = [](Expr s, Expr st, Expr et, Expr d) { return _add(s, d); };
    return _red(win, _f32(0), acc);
}

Op _WindowAvg(string query_name, _sym in, int64_t w)
{
    auto window = in[_win(-w, 0)];
    auto window_sym = _sym("win", window);
    auto count = _Count(window_sym);
    auto count_sym = _sym(query_name + "_count", count);
    auto sum = _Sum(window_sym);
    auto sum_sym = _sym(query_name + "_sum", sum);
    auto avg = sum_sym / count_sym;
    auto avg_sym = _sym("avg", avg);
    auto wc_op = _op(
        _iter(0, w),
        Params{ in },
        SymTable{ {window_sym, window}, {count_sym, count}, {sum_sym, sum}, {avg_sym, avg} },
        _true(),
        avg_sym);
    return wc_op;
}

Op _Join(_sym left, _sym right)
{
    auto e_left = left[_pt(0)];
    auto e_left_sym = _sym("left", e_left);
    auto e_right = right[_pt(0)];
    auto e_right_sym = _sym("right", e_right);
    auto norm = e_left_sym - e_right_sym;
    auto norm_sym = _sym("norm", norm);
    auto left_exist = _exists(e_left_sym);
    auto right_exist = _exists(e_right_sym);
    auto join_cond = left_exist && right_exist;
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

Op _SelectSub(_sym in, _sym avg)
{
    auto e = in[_pt(0)];
    auto e_sym = _sym("e", e);
    auto res = e_sym - avg;
    auto res_sym = _sym("res", res);
    auto sel_op = _op(
        _iter(0, 1),
        Params{in, avg},
        SymTable{{e_sym, e}, {res_sym, res}},
        _exists(e_sym),
        res_sym);
    return sel_op;
}

Op _SelectDiv(_sym in, _sym std)
{
    auto e = in[_pt(0)];
    auto e_sym = _sym("e", e);
    auto res = e_sym / std;
    auto res_sym = _sym("res", res);
    auto sel_op = _op(
        _iter(0, 1),
        Params{in, std},
        SymTable{{e_sym, e}, {res_sym, res}},
        _exists(e_sym),
        res_sym);
    return sel_op;
}

Expr _Average(_sym win)
{
    auto acc = [](Expr s, Expr st, Expr et, Expr d) {
        auto sum = _get(s, 0);
        auto count = _get(s, 1);
        return _new(vector<Expr>{_add(sum, d), _add(count, _f32(1))});
    };
    return _red(win, _new(vector<Expr>{_f32(0), _f32(0)}), acc);
}

Expr _StdDev(_sym win)
{
    auto acc = [](Expr s, Expr st, Expr et, Expr d) {
        auto sum = _get(s, 0);
        auto count = _get(s, 1);
        return _new(vector<Expr>{_add(sum, _mul(d, d)), _add(count, _f32(1))});
    };
    return _red(win, _new(vector<Expr>{_f32(0), _f32(0)}), acc);
}

Op _Norm(string query_name, _sym in, int64_t len)
{
    auto inwin = in[_win(-len, 0)];
    auto inwin_sym = _sym("inwin", inwin);

    // avg state
    auto avg_state = _Average(inwin_sym);
    auto avg_state_sym = _sym(query_name + "_avg_state", avg_state);

    // avg value
    auto avg = _div(_get(avg_state_sym, 0), _get(avg_state_sym, 1));
    auto avg_sym = _sym("avg", avg);

    // avg join
    auto avg_op = _SelectSub(inwin_sym, avg_sym);
    auto avg_op_sym = _sym(query_name + "_avgop", avg_op);

    // stddev state
    auto std_state = _StdDev(avg_op_sym);
    auto std_state_sym = _sym(query_name + "_stddev_state", std_state);

    // stddev value
    auto std = _sqrt(_div(_get(std_state_sym, 0), _get(std_state_sym, 1)));
    auto std_sym = _sym("std", std);

    // std join
    auto std_op = _SelectDiv(avg_op_sym, std_sym);
    auto std_op_sym = _sym(query_name + "_stdop", std_op);

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

Op _MovingSum(_sym in, int64_t dur, int64_t w)
{
    auto e = in[_pt(0)];
    auto e_sym = _sym("e", e);
    auto p = in[_pt(-w)];
    auto p_sym = _sym("p", p);
    auto out = _out(types::INT32, _iter(0, dur));
    auto o = out[_pt(-dur)];
    auto o_sym = _sym("o", o);
    auto p_val = _ifelse(_exists(p_sym), p_sym, _i32(0));
    auto p_val_sym = _sym("p_val", p_val);
    auto o_val = _ifelse(_exists(o_sym), o_sym, _i32(0));
    auto o_val_sym = _sym("o_val", o_val);
    auto res = (e_sym + o_val_sym) - p_val_sym;
    auto res_sym = _sym("res", res);
    auto sel_op = _op(
        _iter(0, dur),
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

Op _Pair(_sym in, int64_t iperiod)
{
    auto ev = in[_pt(0)];
    auto ev_sym = _sym("ev", ev);
    auto sv = in[_pt(-iperiod)];
    auto sv_sym = _sym("sv", sv);
    auto beat = _beat(_iter(0, iperiod));
    auto et = _cast(types::FLOAT32, beat[_pt(0)]);
    auto et_sym = _sym("et", et);
    auto st = et_sym - _f32(iperiod);
    auto st_sym = _sym("st", st);
    auto res = _new(vector<Expr>{st_sym, sv_sym, et_sym, ev_sym});
    auto res_sym = _sym("res", res);
    auto pair_op = _op(
        _iter(0, iperiod),
        Params{in, beat},
        SymTable{
            {st_sym, st},
            {sv_sym, sv},
            {et_sym, et},
            {ev_sym, ev},
            {res_sym, res},
        },
        _exists(sv_sym) && _exists(ev_sym),
        res_sym);
    return pair_op;
}

Op _Interpolate(_sym in, int64_t operiod)
{
    auto e = in[_pt(0)];
    auto e_sym = _sym("e", e);
    auto beat = _beat(_iter(0, operiod));
    auto t = _cast(types::FLOAT32, beat[_pt(0)]);
    auto t_sym = _sym("t", t);
    auto st = e_sym << 0;
    auto sv = e_sym << 1;
    auto et = e_sym << 2;
    auto ev = e_sym << 3;
    auto res = (((ev - sv) * (t_sym - st)) / (et - st)) + sv;
    auto res_sym = _sym("res", res);
    auto inter_op = _op(
        _iter(0, operiod),
        Params{in, beat},
        SymTable{
            {e_sym, e},
            {t_sym, t},
            {res_sym, res},
        },
        _exists(e_sym),
        res_sym);
    return inter_op;
}

Op _Resample(string query_name, _sym in, int64_t iperiod, int64_t operiod)
{
    auto win_size = lcm(iperiod, operiod);
    auto win = in[_win(-win_size, 0)];
    auto win_sym = _sym("win", win);
    auto pair = _Pair(win_sym, iperiod);
    auto pair_sym = _sym(query_name + "_pair", pair);
    auto inter = _Interpolate(pair_sym, operiod);
    auto inter_sym = _sym(query_name + "_inter", inter);
    auto resample_op = _op(
        _iter(0, win_size),
        Params{in},
        SymTable{
            {win_sym, win},
            {pair_sym, pair},
            {inter_sym, inter},
        },
        _true(),
        inter_sym);
    return resample_op;
}
