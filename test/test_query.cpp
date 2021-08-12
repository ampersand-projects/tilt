#include "test_query.h"

using namespace tilt::tilder;

Op _Select(Sym in, function<Expr(Expr)> sel_expr)
{
    auto e = _elem(in, _pt(0));
    auto e_sym = e->sym("e");
    auto e_val = _read(e_sym);
    auto e_val_sym = e_val->sym("e_val");
    auto sel = sel_expr(_get(e_val_sym, 0));
    auto sel_sym = sel->sym("sel");
    auto sel_op = _op(
        _iter(0, 1),
        Params{ in },
        SymTable{ {e_sym, e}, {e_val_sym, e_val}, {sel_sym, sel} },
        _exists(e_sym),
        sel_sym);
    return sel_op;
}

Op _MovingSum(Sym in, int64_t dur, int64_t w)
{
    auto e = _elem(in, _pt(0));
    auto e_sym = e->sym("e");
    auto e_val = _read(e_sym);
    auto e_val_sym = e_val->sym("e_val");
    auto p = _elem(in, _pt(-w));
    auto p_sym = p->sym("p");
    auto p_val = _ifelse(_exists(p_sym), _read(p_sym), _const(in->type.dtype.btype, 0));
    auto p_val_sym = p_val->sym("p_val");
    auto o = _elem(_out(Type(in->type.dtype, _iter(0, -1))), _pt(-dur));
    auto o_sym = o->sym("o");
    auto o_val = _ifelse(_exists(o_sym), _read(o_sym), _const(in->type.dtype.btype, 0));
    auto o_val_sym = o_val->sym("o_val");
    auto res = _sub(_add(e_val_sym, o_val_sym), p_val_sym);
    auto res_sym = res->sym("res");
    auto mov_op = _op(
        _iter(0, dur),
        Params{ in },
        SymTable{
            {e_sym, e},
            {e_val_sym, e_val},
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
