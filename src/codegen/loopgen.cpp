#include "tilt/codegen/loopgen.h"

#include <string>
#include <unordered_set>

using namespace tilt;
using namespace std;

Indexer& LoopGen::create_idx(const SymPtr reg, const Point pt)
{
    auto& pt_idx_map = ctx().pt_idx_maps[reg];
    if (pt_idx_map.find(pt) == pt_idx_map.end()) {
        auto idx = make_shared<Index>("i" + to_string(pt.offset) + "_" + reg->name);
        ctx().loop->idxs.push_back(idx);

        auto& idx_state = ctx().loop->states[idx];
        idx_state.base = make_shared<Index>("i" + to_string(pt.offset) + "_base_" + reg->name);
        idx_state.init = make_shared<AllocIndex>(make_shared<GetStartIdx>(reg));

        auto time_expr = make_shared<Add>(ctx().loop->t, make_shared<TConst>(pt.offset));
        auto adv_expr = make_shared<Advance>(reg, idx_state.base, time_expr);
        idx_state.update = adv_expr;
        pt_idx_map[pt] = idx;
    }

    return pt_idx_map[pt];
}

void LoopGen::build_loop()
{
    auto loop = ctx().loop;
    auto name = loop->name;

    /* Arguments */
    auto t_start = make_shared<Time>("t_start");
    auto t_end = make_shared<Time>("t_end");
    auto out_reg_arg = make_shared<Region>(name, ctx().op->type);
    loop->inputs.push_back(t_start);
    loop->inputs.push_back(t_end);
    loop->inputs.push_back(out_reg_arg);
    for (auto& in : ctx().op->inputs) {
        SymPtr in_reg = make_shared<Region>(in->name, in->type);
        loop->inputs.push_back(in_reg);
        map_sym(in) = in_reg;
    }

    loop->t = make_shared<Time>("t");
    auto t_base = make_shared<Time>("t_base");
    loop->states[loop->t].base = t_base;
    loop->states[loop->t].init = t_start;

    loop->output = make_shared<Region>("output", ctx().op->type);
    auto output_base = make_shared<Region>("output_base", ctx().op->type);
    loop->states[loop->output].base = output_base;
    loop->states[loop->output].init = out_reg_arg;

    auto pred_expr = eval(ctx().op->pred);
    auto out_expr = eval(ctx().op->output);

    map<Indexer, SymPtr> edge_idxs;
    for (const auto& [reg, pt_idx_map]: ctx().pt_idx_maps) {
        auto& tail_idx = pt_idx_map.begin()->second;
        auto& head_idx = pt_idx_map.rbegin()->second;
        edge_idxs[tail_idx] = reg;
        edge_idxs[head_idx] = reg;
    }

    ExprPtr delta = nullptr;
    for (const auto& [idx, reg]: edge_idxs) {
        const auto& base_idx = loop->states[idx].base;
        auto next_idx_expr = make_shared<Next>(reg, base_idx);
        auto next_time_expr = make_shared<GetTime>(next_idx_expr);
        auto cur_time_expr = make_shared<GetTime>(base_idx);
        auto diff_expr = make_shared<Sub>(next_time_expr, cur_time_expr);
        if (delta) {
            delta = make_shared<Min>(delta, diff_expr);
        } else {
            delta = diff_expr;
        }
    }

    auto t_incr = make_shared<Max>(make_shared<TConst>(ctx().op->iter.period), delta);
    loop->states[loop->t].update = make_shared<Add>(t_base, t_incr);

    loop->exit_cond = make_shared<GreaterThan>(loop->t, t_end);

    auto true_body = out_expr->type.isLStream() ? out_expr :
        make_shared<CommitData>(output_base, loop->t, out_expr);
    auto false_body = make_shared<CommitNull>(output_base, loop->t);
    loop->states[loop->output].update = make_shared<IfElse>(pred_expr, true_body, false_body);
}

ExprPtr LoopGen::visit(const Symbol& symbol)
{
    shared_ptr<Symbol> tmp_sym(const_cast<Symbol*>(&symbol), [](Symbol*) {});
    return map_sym(tmp_sym);
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
    auto& sym_ptr = ctx().sym_ref_map[exists.sym];
    return make_shared<Exists>(sym_ptr);
}

ExprPtr LoopGen::visit(const Equals& equals)
{
    return make_shared<Equals>(eval(equals.a), eval(equals.b));
}

ExprPtr LoopGen::visit(const Not& not_expr)
{
    return make_shared<Not>(eval(not_expr.a));
}

ExprPtr LoopGen::visit(const And& and_expr)
{
    return make_shared<And>(eval(and_expr.a), eval(and_expr.b));
}

ExprPtr LoopGen::visit(const Or& or_expr)
{
    return make_shared<And>(eval(or_expr.a), eval(or_expr.b));
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
    return make_shared<LessThan>(eval(lt.a), eval(lt.b));
}

ExprPtr LoopGen::visit(const LessThanEqual& lte)
{
    return make_shared<LessThanEqual>(eval(lte.a), eval(lte.b));
}

ExprPtr LoopGen::visit(const GreaterThan& gt)
{
    return make_shared<LessThanEqual>(eval(gt.a), eval(gt.b));
}

ExprPtr LoopGen::visit(const SubLStream& subls)
{
    auto& reg = map_sym(subls.lstream);
    auto& start_idx = create_idx(reg, subls.win.start);
    auto& end_idx = create_idx(reg, subls.win.end);
    return make_shared<MakeRegion>(reg, start_idx, end_idx);
}

ExprPtr LoopGen::visit(const Element& elem)
{
    auto& reg = map_sym(elem.lstream);
    auto& idx = create_idx(reg, elem.pt);
    auto fetch = make_shared<Fetch>(reg, idx);
    auto ref_sym = fetch->GetSym(ctx().sym->name + "_ptr");
    sym(ref_sym) = fetch;
    ctx().sym_ref_map[ctx().sym] = ref_sym;
    return make_shared<Load>(ref_sym);
}

Looper LoopGen::Build(SymPtr sym, const Op* op)
{
    auto loop = make_shared<Loop>(sym);
    LoopGenCtx ctx(sym, op, loop);
    LoopGen loopgen(ctx);
    loopgen.build_loop();
    return ctx.loop;
}

ExprPtr LoopGen::visit(const Op& op)
{
    auto child_op = &op;
    auto child_loop = LoopGen::Build(ctx().sym, child_op);
    auto parent_op = ctx().op;
    auto parent_loop = ctx().loop;

    auto t_start = parent_loop->states[parent_loop->t].base;
    auto t_end = parent_loop->t;

    SymPtr out_sym;
    if (parent_op->output == ctx().sym) {
        out_sym = parent_loop->states[parent_loop->output].base;
    } else {
        auto size = make_shared<Sub>(t_end, t_start);
        auto out_reg = make_shared<AllocRegion>(op.type, size);
        out_sym = out_reg->GetSym(ctx().sym->name);
        parent_loop->syms[out_sym] = out_reg;
    }

    vector<ExprPtr> args = {t_start, t_end, out_sym};
    for (const auto& input: child_op->inputs) {
        args.push_back(eval(input));
    }
    return make_shared<Call>(child_loop, move(args));
}

ExprPtr LoopGen::visit(const AggExpr&) { return nullptr; }