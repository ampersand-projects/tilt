#include <string>
#include <unordered_set>

#include "tilt/pass/codegen/loopgen.h"

using namespace tilt;
using namespace tilt::tilder;
using namespace std;

Expr LoopGen::get_timer(const Point pt)
{
    auto t_base = ctx().loop->state_bases[ctx().loop->t];
    return _add(t_base, _ts(pt.offset));
}

void LoopGen::build_tloop(function<Expr()> true_body, function<Expr()> false_body)
{
    auto loop = ctx().loop;
    auto name = loop->name;

    // Loop function arguments
    auto t_start = _time("t_start");
    auto t_end = _time("t_end");
    auto out_arg = _sym(name, ctx().loop->type);
    loop->inputs.push_back(t_start);
    loop->inputs.push_back(t_end);
    loop->inputs.push_back(out_arg);
    for (auto& in : ctx().op->inputs) {
        auto in_reg = _sym(in->name, in->type);
        set_sym(in, in_reg);
        if (!in_reg->type.is_beat()) {
            loop->inputs.push_back(in_reg);
        }
    }

    // Create loop counter
    auto t_base = _time("t_base");
    set_expr(t_base, t_start);
    loop->t = _time("t");
    loop->state_bases[loop->t] = t_base;

    // Loop exit condition
    loop->exit_cond = _gte(t_base, t_end);

    // Create loop return value
    auto output_base = _sym("output_base", ctx().loop->type);
    set_expr(output_base, out_arg);
    loop->output = _sym("output", ctx().loop->type);
    loop->state_bases[loop->output] = output_base;

    // Evaluate loop body
    auto pred_expr = eval(ctx().op->pred);
    eval(ctx().op->output);

    // Loop counter update expression
    set_expr(loop->t, _add(get_timer(_pt(0)), _ts(ctx().op->iter.period)));

    // Create loop output
    set_expr(loop->output, _ifelse(pred_expr, true_body(), false_body()));
}

void LoopGen::build_loop()
{
    auto true_body = [&]() -> Expr {
        auto loop = ctx().loop;
        auto t_base = loop->state_bases[loop->t];
        auto output_base = loop->state_bases[loop->output];
        auto out_expr = get_sym(ctx().op->output);

        // Update loop output:
        //      1. Outer loop returns the output region of the inner loop
        //      2. Inner loop updates the output region
        if (out_expr->type.is_val()) {
            auto new_reg = _commit_data(output_base, t_base);
            auto new_reg_sym = _sym("new_reg", new_reg);
            set_expr(new_reg_sym, new_reg);

            auto dptr = _fetch(new_reg_sym, t_base);
            return _write(new_reg_sym, dptr, out_expr);
        } else {
            return out_expr;
        }
    };

    auto false_body = [&]() -> Expr {
        auto loop = ctx().loop;
        auto output_base = loop->state_bases[loop->output];
        return _commit_null(output_base, loop->t);
    };

    build_tloop(true_body, false_body);
}

Expr LoopGen::visit(const Symbol& symbol) { return get_sym(symbol); }

Expr LoopGen::visit(const Out& out)
{
    auto out_reg = ctx().loop->inputs[2];
    set_sym(out, out_reg);
    return out_reg;
}

Expr LoopGen::visit(const Beat& beat) { return _beat(beat); }

Expr LoopGen::visit(const IfElse& ifelse)
{
    auto cond = eval(ifelse.cond);
    auto true_body = eval(ifelse.true_body);
    auto false_body = eval(ifelse.false_body);
    return _ifelse(cond, true_body, false_body);
}

Expr LoopGen::visit(const Select& select)
{
    auto cond = eval(select.cond);
    auto true_body = eval(select.true_body);
    auto false_body = eval(select.false_body);
    return _sel(cond, true_body, false_body);
}

Expr LoopGen::visit(const Call& call)
{
    vector<Expr> args;
    for (const auto& arg : call.args) {
        args.push_back(eval(arg));
    }
    return _call(call.name, call.type, std::move(args));
}

Expr LoopGen::visit(const Exists& exists)
{
    eval(exists.sym);
    auto s = get_sym(exists.sym);
    if (s->type.is_beat() || s->type == Type(types::TIME)) {
        return _true();
    } else if (s->type.is_val()) {
        auto ptr_sym = get_ref(exists.sym);
        return _exists(ptr_sym);
    } else {
        auto st = _get_start_time(s);
        auto et = _get_end_time(s);
        auto win_ptr = _fetch(s, st);
        auto win_ptr_sym = _sym(s->name + "_ptr", win_ptr);
        set_expr(win_ptr_sym, win_ptr);
        return _or(_not(_eq(st, et)), _exists(win_ptr_sym));
    }
}

Expr LoopGen::visit(const New& new_expr)
{
    vector<Expr> input_vals;
    for (const auto& input : new_expr.inputs) {
        input_vals.push_back(eval(input));
    }
    return _new(std::move(input_vals));
}

Expr LoopGen::visit(const Get& get) { return _get(eval(get.input), get.n); }

Expr LoopGen::visit(const ConstNode& cnst) { return _const(cnst); }

Expr LoopGen::visit(const Cast& e) { return _cast(e.type.dtype, eval(e.arg)); }

Expr LoopGen::visit(const NaryExpr& e)
{
    vector<Expr> args;
    for (auto arg : e.args) {
        args.push_back(eval(arg));
    }
    return make_shared<NaryExpr>(e.type.dtype, e.op, std::move(args));
}

Expr LoopGen::visit(const SubLStream& subls)
{
    eval(subls.lstream);
    auto& reg = get_sym(subls.lstream);
    if (reg->type.is_beat()) {
        return reg;
    } else {
        auto st = _add(ctx().loop->t, _ts(subls.win.start.offset));
        auto et = _add(ctx().loop->t, _ts(subls.win.end.offset));
        return _make_reg(reg, st, et);
    }
}

Expr LoopGen::visit(const Element& elem)
{
    eval(elem.lstream);
    auto& reg = get_sym(elem.lstream);

    if (reg->type.is_beat()) {
        throw runtime_error("Not implemented");
    } else {
        auto time = get_timer(elem.pt);
        auto ptr = _fetch(reg, time);
        auto ptr_sym = _sym(ctx().sym->name + "_ptr", ptr);
        set_expr(ptr_sym, ptr);
        set_ref(ctx().sym, ptr_sym);
        return _read(ptr_sym);
    }
}

Expr LoopGen::visit(const OpNode& op)
{
    auto inner_op = &op;
    auto inner_loop = LoopGen::Build(ctx().sym, inner_op);
    auto outer_op = ctx().op;
    auto outer_loop = ctx().loop;

    outer_loop->inner_loops.push_back(inner_loop);

    auto t_start = _sub(ctx().loop->t, _ts(outer_op->iter.period));
    auto t_end = ctx().loop->t;

    vector<Expr> inputs;
    Val size_expr = _ts(10000);
    for (const auto& input : inner_op->inputs) {
        auto input_val = eval(input);
        if (!input_val->type.is_beat()) {
            inputs.push_back(input_val);
        }
        if (!input_val->type.is_val()) {
            if (input_val->type.is_beat()) {
                auto period = _ts(inner_op->iter.period);
                auto beat = _ts(input_val->type.iter.period);
                size_expr = _add(size_expr, _div(period, beat));
            } else {
                auto start = _get_start_time(input_val);
                auto end = _get_end_time(input_val);
                size_expr = _add(size_expr, _sub(end, start));
            }
        }
    }

    Sym out_sym;
    if (outer_op->output == ctx().sym) {
        out_sym = outer_loop->state_bases[outer_loop->output];
    } else if (outer_op->aux.find(ctx().sym) != outer_op->aux.end()) {
        out_sym = get_sym(outer_op->aux.at(ctx().sym));
    } else {
        auto out_reg = _alloc_reg(op.type, size_expr, t_start);
        out_sym = _sym(ctx().sym->name + "_reg", out_reg);
        set_expr(out_sym, out_reg);
    }

    vector<Expr> args = {t_start, t_end, out_sym};
    for (const auto& input : inputs) {
        args.push_back(input);
    }
    return _call(inner_loop->get_name(), inner_loop->type, std::move(args));
}

Expr LoopGen::visit(const Reduce& red)
{
    auto e = _elem(red.lstream, _pt(0));
    auto e_sym = _sym("e", e);
    auto red_op = _op(
        _iter(0, red.lstream->type.iter.period),
        Params{red.lstream},
        SymTable{
            {e_sym, e}
        },
        _exists(e_sym),
        e_sym);

    auto red_loop = _loop(ctx().sym);
    LoopGenCtx new_ctx(ctx().sym, red_op.get(), red_loop);

    auto& old_ctx = switch_ctx(new_ctx);

    auto true_body = [&]() -> Expr {
        auto loop = ctx().loop;
        auto output_base = loop->state_bases[loop->output];
        auto t = loop->t;
        auto t_base = loop->state_bases[t];
        auto out_sym = get_sym(ctx().op->output);
        return eval(red.acc(output_base, t_base, t, out_sym));
    };

    auto false_body = [&]() -> Expr {
        auto loop = ctx().loop;
        auto output_base = loop->state_bases[loop->output];
        return output_base;
    };
    build_tloop(true_body, false_body);
    switch_ctx(old_ctx);

    auto outer_loop = ctx().loop;
    outer_loop->inner_loops.push_back(red_loop);

    auto red_input = eval(red.lstream);
    auto t_start = _get_start_time(red_input);
    auto t_end = _get_end_time(red_input);
    vector<Expr> args = { t_start, t_end, eval(red.state), red_input };
    return _call(red_loop->get_name(), red_loop->type, args);
}

Loop LoopGen::Build(Sym sym, const OpNode* op)
{
    auto loop = _loop(sym);
    LoopGenCtx ctx(sym, op, loop);
    LoopGen loopgen(std::move(ctx));
    loopgen.build_loop();
    return loopgen.ctx().loop;
}
