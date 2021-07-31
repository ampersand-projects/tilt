#include <string>
#include <unordered_set>

#include "tilt/codegen/loopgen.h"

using namespace tilt;
using namespace tilt::tilder;
using namespace std;

Indexer& LoopGen::create_idx(const Sym reg, const Point pt)
{
    auto& pt_idx_map = ctx().pt_idx_maps[reg];
    if (pt_idx_map.find(pt) == pt_idx_map.end()) {
        auto idx_base = _idx("i" + to_string(pt.offset) + "_base_" + reg->name);
        assign(idx_base, _alloc_idx(_get_start_idx(reg)));

        auto idx = _idx("i" + to_string(pt.offset) + "_" + reg->name);
        auto time_expr = _add(ctx().loop->t, _ts(pt.offset));
        auto adv_expr = _adv(reg, idx_base, time_expr);
        assign(idx, adv_expr);
        pt_idx_map[pt] = idx;

        ctx().loop->idxs.push_back(idx);
        ctx().loop->state_bases[idx] = idx_base;
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
        Sym in_reg = _reg(in->name, in->type);
        loop->inputs.push_back(in_reg);
        sym(in) = in_reg;
    }

    // Create loop counter
    auto t_base = _time("t_base");
    assign(t_base, t_start);
    loop->t = _time("t");
    loop->state_bases[loop->t] = t_base;

    // Loop exit condition
    loop->exit_cond = _eq(t_base, t_end);

    // Create loop return value
    auto output_base = _reg("output_base", ctx().op->type);
    assign(output_base, out_arg);
    loop->output = _reg("output", ctx().op->type);
    loop->state_bases[loop->output] = output_base;

    // Evaluate loop body
    auto pred_expr = eval(ctx().op->pred);
    auto out_expr = eval(ctx().op->output);

    // Populate edge indices on the inputs
    map<Indexer, Sym> edge_idxs;
    for (const auto& [reg, pt_idx_map] : ctx().pt_idx_maps) {
        auto& tail_idx = pt_idx_map.begin()->second;
        auto& head_idx = pt_idx_map.rbegin()->second;
        edge_idxs[tail_idx] = reg;
        edge_idxs[head_idx] = reg;
    }

    // Expression to calculate loop counter shift
    Expr delta = nullptr;
    for (const auto& [idx, reg] : edge_idxs) {
        const auto& base_idx = loop->state_bases.at(idx);
        auto next_time_expr = _next_time(reg, base_idx);
        auto cur_time_expr = _get_time(base_idx);
        auto diff_expr = _sub(next_time_expr, cur_time_expr);
        if (delta) {
            delta = _min(delta, diff_expr);
        } else {
            delta = diff_expr;
        }
    }

    // Loop counter update expression
    auto p = _ts(ctx().op->iter.period);
    auto t_incr = _max(p, _mul(_div(delta, p), p));
    assign(loop->t, _min(t_end, _add(t_base, t_incr)));

    // Update loop output:
    //      1. Outer loop returns the output region of the inner loop
    //      2. Inner loop updates the output region
    Expr true_body = nullptr;
    if (out_expr->type.is_valtype()) {
        auto new_reg = _commit_data(output_base, loop->t);
        auto new_reg_sym = new_reg->sym("new_reg");
        assign(new_reg_sym, new_reg);

        auto idx = _get_end_idx(new_reg_sym);
        auto dptr = _fetch(new_reg_sym, idx);
        true_body = _store(new_reg_sym, dptr, out_expr);
    } else {
        true_body = out_expr;
    }
    auto false_body = _commit_null(output_base, loop->t);
    assign(loop->output, _sel(pred_expr, true_body, false_body));
}

Expr LoopGen::visit(const Symbol& symbol)
{
    return sym(get_sym(symbol));
}

Expr LoopGen::visit(const IfElse& ifelse)
{
    auto cond = eval(ifelse.cond);
    auto true_body = eval(ifelse.true_body);
    auto false_body = eval(ifelse.false_body);
    return _sel(cond, true_body, false_body);
}

Expr LoopGen::visit(const Exists& exists)
{
    eval(exists.sym);
    auto& sym_ptr = sym_ref(exists.sym);
    return _exists(sym_ptr);
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

Expr LoopGen::visit(const NaryExpr& e)
{
    switch (e.op) {
        case MathOp::ADD: return _add(eval(e.arg(0)), eval(e.arg(1)));
        case MathOp::SUB: return _sub(eval(e.arg(0)), eval(e.arg(1)));
        case MathOp::MUL: return _mul(eval(e.arg(0)), eval(e.arg(1)));
        case MathOp::DIV: return _div(eval(e.arg(0)), eval(e.arg(1)));
        case MathOp::MOD: return _mod(eval(e.arg(0)), eval(e.arg(1)));
        case MathOp::MAX: return _max(eval(e.arg(0)), eval(e.arg(1)));
        case MathOp::MIN: return _min(eval(e.arg(0)), eval(e.arg(1)));
        case MathOp::SQRT: return _sqrt(eval(e.arg(0)));
        case MathOp::POW: return _pow(eval(e.arg(0)), eval(e.arg(1)));
        case MathOp::CEIL: return _ceil(eval(e.arg(0)));
        case MathOp::FLOOR: return _floor(eval(e.arg(0)));
        case MathOp::ABS: return _abs(eval(e.arg(0)));
        case MathOp::EQ: return _eq(eval(e.arg(0)), eval(e.arg(1)));
        case MathOp::NOT: return _not(eval(e.arg(0)));
        case MathOp::AND: return _and(eval(e.arg(0)), eval(e.arg(1)));
        case MathOp::OR: return _or(eval(e.arg(0)), eval(e.arg(1)));
        case MathOp::LT: return _lt(eval(e.arg(0)), eval(e.arg(1)));
        case MathOp::LTE: return _lte(eval(e.arg(0)), eval(e.arg(1)));
        case MathOp::GT: return _gt(eval(e.arg(0)), eval(e.arg(1)));
        case MathOp::GTE: return _gte(eval(e.arg(0)), eval(e.arg(1)));
        default: throw std::runtime_error("Invalid math operation"); break;
    }
}

Expr LoopGen::visit(const SubLStream& subls)
{
    auto& reg = sym(subls.lstream);
    auto& start_idx = create_idx(reg, subls.win.start);
    auto& end_idx = create_idx(reg, subls.win.end);
    return _make_reg(reg, start_idx, end_idx);
}

Expr LoopGen::visit(const Element& elem)
{
    auto& reg = sym(elem.lstream);
    auto& idx = create_idx(reg, elem.pt);
    auto fetch = _fetch(reg, idx);
    auto ref_sym = fetch->sym(ctx().sym->name + "_ptr");
    assign(ref_sym, fetch);
    sym_ref(ctx().sym) = ref_sym;
    return _load(ref_sym);
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
    Val size_expr = _u32(1);
    for (const auto& input : inner_op->inputs) {
        auto input_val = eval(input);
        inputs.push_back(input_val);
        auto start = _get_idx(_get_start_idx(input_val));
        auto end = _get_idx(_get_end_idx(input_val));
        size_expr = _add(size_expr, _sub(end, start));
    }

    Sym out_sym;
    if (outer_op->output == ctx().sym) {
        out_sym = outer_loop->state_bases[outer_loop->output];
    } else {
        auto out_reg = _alloc_reg(op.type, size_expr, t_start);
        out_sym = out_reg->sym(ctx().sym->name + "_reg");
        assign(out_sym, out_reg);
    }

    vector<Expr> args = {t_start, t_end, out_sym};
    for (const auto& input : inputs) {
        args.push_back(input);
    }
    return _call(inner_loop, move(args));
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
    assign(output_base, out_arg);
    agg_loop->output = _sym("output", aggexpr.type);
    agg_loop->state_bases[agg_loop->output] = output_base;
    auto acc_expr = eval(aggexpr.acc(output_base, sym(aggexpr.op->output)));
    auto output_update = _sel(eval(aggexpr.op->pred), acc_expr, output_base);
    ASSERT(output_update->type == aggexpr.type);
    assign(agg_loop->output, output_update);

    switch_ctx(old_ctx);

    auto outer_loop = ctx().loop;
    outer_loop->inner_loops.push_back(agg_loop);
    auto t_start = outer_loop->state_bases[outer_loop->t];
    auto t_end = outer_loop->t;

    vector<Expr> args = { t_start, t_end, eval(aggexpr.init) };
    for (const auto& input : aggexpr.op->inputs) {
        args.push_back(eval(input));
    }
    return _call(agg_loop, args);
}

Looper LoopGen::Build(Sym sym, const OpNode* op)
{
    auto loop = _loop(sym);
    LoopGenCtx ctx(sym, op, loop);
    LoopGen loopgen(move(ctx));
    loopgen.build_loop();
    return loopgen.ctx().loop;
}
