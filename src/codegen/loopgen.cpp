#include "tilt/codegen/loopgen.h"

#include <string>
#include <unordered_set>

using namespace tilt;
using namespace std;

Indexer& LoopGen::create_idx(const SymPtr reg, const Point pt)
{
    auto& pt_idx_map = ctx().pt_idx_maps[reg];
    if (pt_idx_map.find(pt) == pt_idx_map.end()) {
        auto idx_base = make_shared<Index>("i" + to_string(pt.offset) + "_base_" + reg->name);
        assign(idx_base, make_shared<AllocIndex>(make_shared<GetStartIdx>(reg)));

        auto idx = make_shared<Index>("i" + to_string(pt.offset) + "_" + reg->name);
        auto time_expr = make_shared<Add>(ctx().loop->t, make_shared<TConst>(pt.offset));
        auto adv_expr = make_shared<Advance>(reg, idx_base, time_expr);
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
    auto t_start = make_shared<Time>("t_start");
    auto t_end = make_shared<Time>("t_end");
    auto out_arg = make_shared<Region>(name, ctx().op->type);
    loop->inputs.push_back(t_start);
    loop->inputs.push_back(t_end);
    loop->inputs.push_back(out_arg);
    for (auto& in : ctx().op->inputs) {
        SymPtr in_reg = make_shared<Region>(in->name, in->type);
        loop->inputs.push_back(in_reg);
        sym(in) = in_reg;
    }

    // Create loop counter
    auto t_base = make_shared<Time>("t_base");
    assign(t_base, t_start);
    loop->t = make_shared<Time>("t");
    loop->state_bases[loop->t] = t_base;

    // Create loop return value
    auto output_base = make_shared<Region>("output_base", ctx().op->type);
    assign(output_base, out_arg);
    loop->output = make_shared<Region>("output", ctx().op->type);
    loop->state_bases[loop->output] = output_base;

    // Evaluate loop body
    auto pred_expr = eval(ctx().op->pred);
    auto out_expr = eval(ctx().op->output);

    // Populate edge indices on the inputs
    map<Indexer, SymPtr> edge_idxs;
    for (const auto& [reg, pt_idx_map]: ctx().pt_idx_maps) {
        auto& tail_idx = pt_idx_map.begin()->second;
        auto& head_idx = pt_idx_map.rbegin()->second;
        edge_idxs[tail_idx] = reg;
        edge_idxs[head_idx] = reg;
    }

    // Expression to calculate loop counter shift
    ExprPtr delta = nullptr;
    for (const auto& [idx, reg]: edge_idxs) {
        const auto& base_idx = loop->state_bases[idx];
        auto next_time_expr = make_shared<NextTime>(reg, base_idx);
        auto cur_time_expr = make_shared<GetTime>(base_idx);
        auto diff_expr = make_shared<Sub>(next_time_expr, cur_time_expr);
        if (delta) {
            delta = make_shared<Min>(delta, diff_expr);
        } else {
            delta = diff_expr;
        }
    }

    // Loop counter update expression
    auto t_incr = make_shared<Max>(make_shared<TConst>(ctx().op->iter.period), delta);
    assign(loop->t, make_shared<Add>(t_base, t_incr));

    // Loop exit condition
    loop->exit_cond = make_shared<GreaterThan>(loop->t, t_end);

    // Update loop output:
    //      1. Outer loop returns the output region of the inner loop
    //      2. Inner loop updates the output region
    ExprPtr true_body = nullptr;
    if (out_expr->type.isLStream()) {
        true_body = out_expr;
    } else {
        auto new_reg = make_shared<CommitData>(output_base, loop->t);
        auto new_reg_sym = new_reg->GetSym("new_reg");
        assign(new_reg_sym, new_reg);

        auto idx = make_shared<GetEndIdx>(new_reg_sym);
        auto dptr = make_shared<Fetch>(new_reg_sym, idx);
        true_body = make_shared<Store>(new_reg_sym, dptr, out_expr);
    }
    auto false_body = make_shared<CommitNull>(output_base, loop->t);
    assign(loop->output, make_shared<IfElse>(pred_expr, true_body, false_body));
}

ExprPtr LoopGen::visit(const Symbol& symbol)
{
    return sym(get_sym(symbol));
}

ExprPtr LoopGen::visit(const IfElse& ifelse)
{
    auto cond = eval(ifelse.cond);
    auto true_body = eval(ifelse.true_body);
    auto false_body = eval(ifelse.false_body);
    return make_shared<IfElse>(cond, true_body, false_body);
}

ExprPtr LoopGen::visit(const Exists& exists)
{
    eval(exists.sym);
    auto& sym_ptr = sym_ref(exists.sym);
    return make_shared<Exists>(sym_ptr);
}

ExprPtr LoopGen::visit(const Equals& equals)
{
    return make_shared<Equals>(eval(equals.Left()), eval(equals.Right()));
}

ExprPtr LoopGen::visit(const Not& not_expr)
{
    return make_shared<Not>(eval(not_expr.Input()));
}

ExprPtr LoopGen::visit(const And& and_expr)
{
    return make_shared<And>(eval(and_expr.Left()), eval(and_expr.Right()));
}

ExprPtr LoopGen::visit(const Or& or_expr)
{
    return make_shared<And>(eval(or_expr.Left()), eval(or_expr.Right()));
}

ExprPtr LoopGen::visit(const IConst& iconst) { return make_shared<IConst>(iconst); }
ExprPtr LoopGen::visit(const UConst& uconst) { return make_shared<UConst>(uconst); }
ExprPtr LoopGen::visit(const FConst& fconst) { return make_shared<FConst>(fconst); }
ExprPtr LoopGen::visit(const CConst& cconst) { return make_shared<CConst>(cconst); }
ExprPtr LoopGen::visit(const TConst& tconst) { return make_shared<TConst>(tconst); }

ExprPtr LoopGen::visit(const Add& add)
{
    return make_shared<Add>(eval(add.Left()), eval(add.Right()));
}

ExprPtr LoopGen::visit(const Sub& sub)
{
    return make_shared<Sub>(eval(sub.Left()), eval(sub.Right()));
}

ExprPtr LoopGen::visit(const Max& max)
{
    return make_shared<Max>(eval(max.Left()), eval(max.Right()));
}

ExprPtr LoopGen::visit(const Min& min)
{
    return make_shared<Min>(eval(min.Left()), eval(min.Right()));
}

ExprPtr LoopGen::visit(const Now&) { return make_shared<Now>(); }
ExprPtr LoopGen::visit(const True&) { return make_shared<True>(); }
ExprPtr LoopGen::visit(const False&) { return make_shared<True>(); }
ExprPtr LoopGen::visit(const LessThan& lt)
{
    return make_shared<LessThan>(eval(lt.Left()), eval(lt.Right()));
}

ExprPtr LoopGen::visit(const LessThanEqual& lte)
{
    return make_shared<LessThanEqual>(eval(lte.Left()), eval(lte.Right()));
}

ExprPtr LoopGen::visit(const GreaterThan& gt)
{
    return make_shared<LessThanEqual>(eval(gt.Left()), eval(gt.Right()));
}

ExprPtr LoopGen::visit(const SubLStream& subls)
{
    auto& reg = sym(subls.lstream);
    auto& start_idx = create_idx(reg, subls.win.start);
    auto& end_idx = create_idx(reg, subls.win.end);
    return make_shared<MakeRegion>(reg, start_idx, end_idx);
}

ExprPtr LoopGen::visit(const Element& elem)
{
    auto& reg = sym(elem.lstream);
    auto& idx = create_idx(reg, elem.pt);
    auto fetch = make_shared<Fetch>(reg, idx);
    auto ref_sym = fetch->GetSym(ctx().sym->name + "_ptr");
    assign(ref_sym, fetch);
    sym_ref(ctx().sym) = ref_sym;
    return make_shared<Load>(ref_sym);
}

ExprPtr LoopGen::visit(const Op& op)
{
    auto inner_op = &op;
    auto inner_loop = LoopGen::Build(ctx().sym, inner_op);
    auto outer_op = ctx().op;
    auto outer_loop = ctx().loop;

    outer_loop->inner_loops.push_back(inner_loop);

    auto t_start = outer_loop->state_bases[outer_loop->t];
    auto t_end = outer_loop->t;

    vector<ExprPtr> inputs;
    ValExprPtr size_expr = make_shared<UConst>(types::UINT32, 1);
    for (const auto& input: inner_op->inputs) {
        auto input_val = eval(input);
        inputs.push_back(input_val);
        auto start = make_shared<GetIndex>(make_shared<GetStartIdx>(input_val));
        auto end = make_shared<GetIndex>(make_shared<GetEndIdx>(input_val));
        size_expr = make_shared<Add>(size_expr, make_shared<Sub>(end, start));
    }

    SymPtr out_sym;
    if (outer_op->output == ctx().sym) {
        out_sym = outer_loop->state_bases[outer_loop->output];
    } else {
        auto out_reg = make_shared<AllocRegion>(op.type, size_expr, t_start);
        out_sym = out_reg->GetSym(ctx().sym->name + "_reg");
        assign(out_sym, out_reg);
    }

    vector<ExprPtr> args = {t_start, t_end, out_sym};
    for (const auto& input: inputs) {
        args.push_back(input);
    }
    return make_shared<Call>(inner_loop, move(args));
}

ExprPtr LoopGen::visit(const AggExpr& aggexpr)
{
    auto agg_loop = make_shared<Loop>(ctx().sym);
    LoopGenCtx new_ctx(ctx().sym, aggexpr.op.get(), agg_loop);

    auto& old_ctx = switch_ctx(new_ctx);
    build_loop();

    // Redefine output argument and states
    auto out_arg = make_shared<Symbol>(ctx().sym->name, aggexpr.type);
    agg_loop->inputs[2] = out_arg;
    agg_loop->syms.erase(agg_loop->state_bases[agg_loop->output]);
    agg_loop->syms.erase(agg_loop->output);
    agg_loop->state_bases.erase(agg_loop->output);

    auto output_base = make_shared<Symbol>("output_base", aggexpr.type);
    assign(output_base, out_arg);
    agg_loop->output = make_shared<Symbol>("output", aggexpr.type);
    agg_loop->state_bases[agg_loop->output] = output_base;
    auto acc_expr = eval(aggexpr.acc(output_base, sym(aggexpr.op->output)));
    auto output_update = make_shared<IfElse>(eval(aggexpr.op->pred), acc_expr, output_base);
    assert(output_update->type == aggexpr.type);
    assign(agg_loop->output, output_update);

    switch_ctx(old_ctx);

    auto outer_loop = ctx().loop;
    outer_loop->inner_loops.push_back(agg_loop);
    auto t_start = outer_loop->state_bases[outer_loop->t];
    auto t_end = outer_loop->t;

    vector<ExprPtr> args = { t_start, t_end, eval(aggexpr.init) };
    for (const auto& input: aggexpr.op->inputs) {
        args.push_back(eval(input));
    }
    return make_shared<Call>(agg_loop, args);
}
