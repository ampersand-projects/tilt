#include "test_query.h"

using namespace tilt::tilder;

Op _Select(_sym in, function<Expr(Expr)> sel_expr)
{
    auto e = in[_pt(0)];
    auto e_sym = _sym("e", e);
    auto sel = sel_expr(_get(e_sym, 0));
    auto sel_sym = _sym("sel", sel);
    auto sel_op = _op(
        _iter(0, 1),
        Params{ in },
        SymTable{ {e_sym, e}, {sel_sym, sel} },
        _exists(e_sym),
        sel_sym);
    return sel_op;
}

Op _MovingSum(_sym in, int64_t dur, int64_t w)
{
    auto e = in[_pt(0)];
    auto e_sym = _sym("e", e);
    auto p = in[_pt(-w)];
    auto p_sym = _sym("p", p);
    auto p_val = _ifelse(_exists(p_sym), p_sym, _const(in->type.dtype.btype, 0));
    auto p_val_sym = _sym("p_val", p_val);
    auto o = _elem(_out(Type(in->type.dtype, _iter(0, -1))), _pt(-dur));
    auto o_sym = _sym("o", o);
    auto o_val = _ifelse(_exists(o_sym), o_sym, _const(in->type.dtype.btype, 0));
    auto o_val_sym = _sym("o_val", o_val);
    auto res = (e_sym + o_val_sym) - p_val_sym;
    auto res_sym = _sym("res", res);
    auto mov_op = _op(
        _iter(0, dur),
        Params{ in },
        SymTable{
            {e_sym, e},
            {p_sym, p},
            {p_val_sym, p_val},
            {o_sym, o},
            {o_val_sym, o_val},
            {res_sym, res},
        },
        _exists(e_sym),
        res_sym);
    return mov_op;
}
