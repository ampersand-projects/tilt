#include <string>
#include <unordered_set>

#include "tilt/codegen/loopgen.h"

using namespace tilt;
using namespace tilt::tilder;
using namespace std;

Expr LoopGen::get_timer(const Point pt)
{
    auto t_base = ctx().loop->state_bases[ctx().loop->t];
    return _add(_add(t_base, _ts(ctx().op->iter.period)), _ts(pt.offset));
}

Expr get_beat_idx(Sym reg, Expr time)
{
    auto period = _ts(reg->type.iter.period);
    auto offset = _ts(reg->type.iter.offset);

    return _cast(types::INDEX, _div(_sub(time, offset), period));
}

Expr get_beat_time(Sym reg, Expr idx)
{
    auto period = _ts(reg->type.iter.period);
    auto offset = _ts(reg->type.iter.offset);

    return _add(_mul(_cast(types::TIME, idx), period), offset);
}

Index& LoopGen::get_idx(const Sym reg, const Point pt)
{
    auto& pt_idx_map = ctx().pt_idx_maps[reg];
    if (pt_idx_map.find(pt) == pt_idx_map.end()) {
        auto time = get_timer(pt);
        auto idx = _index("i" + to_string(pt.offset) + "_" + reg->name);
        Expr next_ckpt = nullptr;

        if (reg->type.is_beat()) {

            // Index updater
            set_expr(idx, get_beat_idx(reg, time));

            // Index shift expression
            next_ckpt = get_beat_time(reg, _add(idx, _idx(1)));
        } else {
            auto idx_base = _index("i" + to_string(pt.offset) + "_" + reg->name + "_base");

            // Index initializer
            set_expr(idx_base, _get_start_idx(reg));
            ctx().loop->state_bases[idx] = idx_base;

            // Index updater
            auto adv_expr = _adv(reg, idx_base, time);
            set_expr(idx, adv_expr);

            // Index shift expression
            next_ckpt = _get_ckpt(reg, time, idx);
        }

        pt_idx_map[pt] = idx;
        ctx().loop->idxs.push_back(idx);
        ctx().idx_diff_map[idx] = _sub(next_ckpt, time);
    }

    return pt_idx_map[pt];
}

void LoopGen::build_loop()
{
    auto loop = ctx().loop;
    auto name = loop->name;

    // Loop function arguments
    auto t_start = _time("t_start");
    auto t_end = _time("t_end");
    auto out_arg = _reg(name, ctx().op->type);
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
    loop->exit_cond = _eq(t_base, t_end);

    // Create loop return value
    auto output_base = _reg("output_base", ctx().op->type);
    set_expr(output_base, out_arg);
    loop->output = _reg("output", ctx().op->type);
    loop->state_bases[loop->output] = output_base;

    // Evaluate loop body
    auto pred_expr = eval(ctx().op->pred);
    auto out_expr = eval(ctx().op->output);

    // Loop counter update expression
    Expr delta = nullptr;
    for (const auto& [idx, diff_expr] : ctx().idx_diff_map) {
        if (delta) {
            delta = _min(delta, diff_expr);
        } else {
            delta = diff_expr;
        }
    }
    auto t_period = _ts(ctx().op->iter.period);
    auto t_incr = _mul(_div(delta, t_period), t_period);
    set_expr(loop->t, _min(t_end, _add(get_timer(_pt(0)), t_incr)));

    // Update loop output:
    //      1. Outer loop returns the output region of the inner loop
    //      2. Inner loop updates the output region
    Expr true_body = nullptr;
    if (out_expr->type.is_valtype()) {
        auto new_reg = _commit_data(output_base, loop->t);
        auto new_reg_sym = new_reg->sym("new_reg");
        set_expr(new_reg_sym, new_reg);

        auto idx = _get_end_idx(new_reg_sym);
        auto dptr = _fetch(new_reg_sym, loop->t, idx);
        true_body = _write(new_reg_sym, dptr, out_expr);
    } else {
        true_body = out_expr;
    }
    auto false_body = _commit_null(output_base, loop->t);
    set_expr(loop->output, _ifelse(pred_expr, true_body, false_body));
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
    return _call(call.name, call.type, move(args));
}

Expr LoopGen::visit(const Exists& exists)
{
    eval(exists.sym);
    auto reg = get_sym(exists.sym);
    if (reg->type.is_beat()) {
        return _true();
    } else if (reg->type.is_valtype()) {
        auto ptr_sym = get_ref(exists.sym);
        return _exists(ptr_sym);
    } else {
        auto si = _get_start_idx(reg);
        auto ei = _get_end_idx(reg);
        auto st = _get_start_time(reg);
        auto win_ptr = _fetch(reg, st, si);
        auto win_ptr_sym = win_ptr->sym(reg->name + "_ptr");
        set_expr(win_ptr_sym, win_ptr);
        return _or(_not(_eq(si, ei)), _exists(win_ptr_sym));
    }
}

Expr LoopGen::visit(const New& new_expr)
{
    vector<Expr> input_vals;
    for (const auto& input : new_expr.inputs) {
        input_vals.push_back(eval(input));
    }
    return _new(move(input_vals));
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
    return make_shared<NaryExpr>(e.type.dtype, e.op, move(args));
}

Expr LoopGen::visit(const SubLStream& subls)
{
    eval(subls.lstream);
    auto& reg = get_sym(subls.lstream);
    if (reg->type.is_beat()) {
        return reg;
    } else {
        auto st = get_timer(subls.win.start);
        auto& si = get_idx(reg, subls.win.start);
        auto et = get_timer(subls.win.end);
        auto& ei = get_idx(reg, subls.win.end);
        return _make_reg(reg, st, si, et, ei);
    }
}

Expr LoopGen::visit(const Element& elem)
{
    eval(elem.lstream);
    auto& reg = get_sym(elem.lstream);
    auto time = get_timer(elem.pt);
    auto& idx = get_idx(reg, elem.pt);

    if (reg->type.is_beat()) {
        return get_beat_time(reg, idx);
    } else {
        auto ptr = _fetch(reg, time, idx);
        auto ptr_sym = ptr->sym(ctx().sym->name + "_ptr");
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

    auto t_start = outer_loop->state_bases[outer_loop->t];
    auto t_end = outer_loop->t;

    vector<Expr> inputs;
    Val size_expr = _idx(1);
    for (const auto& input : inner_op->inputs) {
        auto input_val = eval(input);
        if (!input_val->type.is_beat()) {
            inputs.push_back(input_val);
        }
        if (!input_val->type.is_valtype()) {
            if (input_val->type.is_beat()) {
                auto period = _idx(inner_op->iter.period);
                auto beat = _idx(input_val->type.iter.period);
                size_expr = _add(size_expr, _div(period, beat));
            } else {
                auto start = _get_start_idx(input_val);
                auto end = _get_end_idx(input_val);
                size_expr = _add(size_expr, _sub(end, start));
            }
        }
    }

    Sym out_sym;
    if (outer_op->output == ctx().sym) {
        out_sym = outer_loop->state_bases[outer_loop->output];
    } else {
        auto out_reg = _alloc_reg(op.type, size_expr, t_start);
        out_sym = out_reg->sym(ctx().sym->name + "_reg");
        set_expr(out_sym, out_reg);
    }

    vector<Expr> args = {t_start, t_end, out_sym};
    for (const auto& input : inputs) {
        args.push_back(input);
    }
    return _call(inner_loop->get_name(), inner_loop->type, move(args));
}

Expr LoopGen::visit(const AggNode& aggexpr)
{
    auto agg_loop = _loop(ctx().sym);
    LoopGenCtx new_ctx(ctx().sym, aggexpr.op.get(), agg_loop);

    auto& old_ctx = switch_ctx(new_ctx);
    build_loop();

    // Redefine output argument and states
    auto out_arg = _sym(ctx().sym->name, aggexpr.type);
    agg_loop->inputs[2] = out_arg;
    agg_loop->syms.erase(agg_loop->state_bases[agg_loop->output]);
    agg_loop->syms.erase(agg_loop->output);
    agg_loop->state_bases.erase(agg_loop->output);

    auto output_base = _sym("output_base", aggexpr.type);
    set_expr(output_base, out_arg);
    agg_loop->output = _sym("output", aggexpr.type);
    agg_loop->state_bases[agg_loop->output] = output_base;
    auto acc_expr = eval(aggexpr.acc(output_base, get_sym(aggexpr.op->output)));
    auto output_update = _ifelse(eval(aggexpr.op->pred), acc_expr, output_base);
    ASSERT(output_update->type == aggexpr.type);
    set_expr(agg_loop->output, output_update);

    switch_ctx(old_ctx);

    auto outer_loop = ctx().loop;
    outer_loop->inner_loops.push_back(agg_loop);
    auto t_start = outer_loop->state_bases[outer_loop->t];
    auto t_end = outer_loop->t;

    vector<Expr> args = { t_start, t_end, eval(aggexpr.init) };
    for (const auto& input : aggexpr.op->inputs) {
        args.push_back(eval(input));
    }
    return _call(agg_loop->get_name(), agg_loop->type, args);
}

Loop LoopGen::Build(Sym sym, const OpNode* op)
{
    auto loop = _loop(sym);
    LoopGenCtx ctx(sym, op, loop);
    LoopGen loopgen(move(ctx));
    loopgen.build_loop();
    return loopgen.ctx().loop;
}
