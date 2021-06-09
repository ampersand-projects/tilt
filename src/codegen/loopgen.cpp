#include "tilt/codegen/loopgen.h"

#include <string>
#include <unordered_set>

using namespace tilt;
using namespace std;

Indexer& LoopGen::create_idx(const SymPtr reg, const Point pt)
{
    auto& pt_idx_map = ctx.pt_idx_maps[reg];
    if (pt_idx_map.find(pt) == pt_idx_map.end()) {
        auto idx = make_shared<Index>("i" + to_string(pt.offset) + "_" + reg->name);
        ctx.loop->idxs.push_back(idx);

        auto& idx_state = ctx.loop->states[idx];
        idx_state.base = make_shared<Index>("i" + to_string(pt.offset) + "_base_" + reg->name);
        idx_state.init = make_shared<AllocIndex>(make_shared<GetStartIdx>(reg));

        auto time_expr = make_shared<Add>(ctx.loop->t, make_shared<TConst>(pt.offset));
        auto adv_expr = make_shared<Advance>(reg, idx_state.base, time_expr);
        idx_state.update = adv_expr;
        pt_idx_map[pt] = idx;
    }

    return pt_idx_map[pt];
}

void LoopGen::build_loop()
{
    auto name = ctx.loop->name;

    /* Arguments */
    auto t_start = make_shared<Time>("t_start");
    auto t_end = make_shared<Time>("t_end");
    auto out_reg_arg = make_shared<Region>(name, ctx.op->type);
    ctx.loop->inputs.push_back(t_start);
    ctx.loop->inputs.push_back(t_end);
    ctx.loop->inputs.push_back(out_reg_arg);
    for (auto& in : ctx.op->inputs) {
        auto in_reg = make_shared<Region>(in->name, in->type);
        ctx.loop->inputs.push_back(in_reg);
        ctx.sym_sym_map[in] = in_reg;
    }

    ctx.loop->t = make_shared<Time>("t");
    auto t_base = make_shared<Time>("t_base");
    ctx.loop->states[ctx.loop->t].base = t_base;
    ctx.loop->states[ctx.loop->t].init = t_start;

    ctx.loop->output = make_shared<Region>("output", ctx.op->type);
    auto output_base = make_shared<Region>("output_base", ctx.op->type);
    ctx.loop->states[ctx.loop->output].base = output_base;
    ctx.loop->states[ctx.loop->output].init = out_reg_arg;

    auto pred_expr = eval(ctx.op->pred);
    auto out_expr = eval(ctx.op->output);

    map<Indexer, SymPtr> edge_idxs;
    for (const auto& [reg, pt_idx_map]: ctx.pt_idx_maps) {
        auto& tail_idx = pt_idx_map.begin()->second;
        auto& head_idx = pt_idx_map.rbegin()->second;
        edge_idxs[tail_idx] = reg;
        edge_idxs[head_idx] = reg;
    }

    ExprPtr delta = nullptr;
    for (const auto& [idx, reg]: edge_idxs) {
        const auto& base_idx = ctx.loop->states[idx].base;
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

    auto t_incr = make_shared<Max>(make_shared<TConst>(ctx.op->iter.period), delta);
    ctx.loop->states[ctx.loop->t].update = make_shared<Add>(t_base, t_incr);

    ctx.loop->exit_cond = make_shared<GreaterThan>(ctx.loop->t, t_end);

    auto true_body = out_expr->type.isLStream() ? out_expr :
        make_shared<CommitData>(output_base, ctx.loop->t, out_expr);
    auto false_body = make_shared<CommitNull>(output_base, ctx.loop->t);
    ctx.loop->states[ctx.loop->output].update = make_shared<IfElse>(pred_expr, true_body, false_body);
}

void LoopGen::Visit(const Symbol& sym)
{
    shared_ptr<Symbol> tmp_sym(const_cast<Symbol*>(&sym), [](Symbol*) {});

    if (ctx.sym_sym_map.find(tmp_sym) != ctx.sym_sym_map.end()) {
        ctx.val = ctx.sym_sym_map[tmp_sym];
    } else {
        auto expr = ctx.op->syms.at(tmp_sym);

        swap(ctx.sym, tmp_sym);
        auto val = eval(expr);
        swap(tmp_sym, ctx.sym);

        auto sym_clone = make_shared<Symbol>(sym);
        ctx.sym_sym_map[tmp_sym] = sym_clone;
        ctx.loop->syms[sym_clone] = val;
        ctx.val = sym_clone;
    }
}

void LoopGen::Visit(const IfElse& ifelse)
{
    auto cond = eval(ifelse.cond);
    auto true_body = eval(ifelse.true_body);
    auto false_body = eval(ifelse.false_body);
    ctx.val = make_shared<IfElse>(cond, true_body, false_body);
}

void LoopGen::Visit(const Exists& exists)
{
    eval(exists.sym);
    auto& sym_ptr = ctx.sym_ref_map[exists.sym];
    ctx.val = make_shared<Exists>(sym_ptr);
}

void LoopGen::Visit(const Equals&) {}
void LoopGen::Visit(const Not&) {}
void LoopGen::Visit(const And&) {}
void LoopGen::Visit(const Or&) {}
void LoopGen::Visit(const IConst& iconst)
{
    ctx.val = make_shared<IConst>(iconst);
}

void LoopGen::Visit(const UConst&) {}
void LoopGen::Visit(const FConst&) {}
void LoopGen::Visit(const BConst&) {}
void LoopGen::Visit(const CConst&) {}
void LoopGen::Visit(const TConst&) {}
void LoopGen::Visit(const Add& add)
{
    ctx.val = make_shared<Add>(eval(add.Left()), eval(add.Right()));
}

void LoopGen::Visit(const Sub&) {}
void LoopGen::Visit(const Max&) {}
void LoopGen::Visit(const Min&) {}
void LoopGen::Visit(const Now&) {}
void LoopGen::Visit(const True&)
{
    ctx.val = make_shared<True>();
}
void LoopGen::Visit(const False&) {}
void LoopGen::Visit(const LessThan&) {}
void LoopGen::Visit(const LessThanEqual&) {}
void LoopGen::Visit(const GreaterThan&) {}

void LoopGen::Visit(const SubLStream& subls)
{
    auto& reg = ctx.sym_sym_map[subls.lstream];
    auto& start_idx = create_idx(reg, subls.win.start);
    auto& end_idx = create_idx(reg, subls.win.end);
    ctx.val = make_shared<MakeRegion>(reg, start_idx, end_idx);
}

void LoopGen::Visit(const Element& elem)
{
    auto& reg = ctx.sym_sym_map[elem.lstream];
    auto& idx = create_idx(reg, elem.pt);
    auto fetch = make_shared<Fetch>(reg, idx);
    auto ref_sym = fetch->GetSym(ctx.sym->name + "_ptr");
    ctx.loop->syms[ref_sym] = fetch;
    ctx.sym_ref_map[ctx.sym] = ref_sym;
    ctx.val = make_shared<Load>(ref_sym);
}

Looper LoopGen::Build(SymPtr sym, const Op* op)
{
    auto loop = make_shared<Loop>(sym);
    LoopGenCtx ctx(sym, op, loop);
    LoopGen loopgen(ctx);
    loopgen.build_loop();
    return ctx.loop;
}

void LoopGen::Visit(const Op& op)
{
    auto child_op = &op;
    auto child_loop = LoopGen::Build(ctx.sym, child_op);
    auto parent_op = ctx.op;
    auto parent_loop = ctx.loop;

    auto t_start = parent_loop->states[parent_loop->t].base;
    auto t_end = parent_loop->t;

    SymPtr out_sym;
    if (parent_op->output == ctx.sym) {
        out_sym = parent_loop->states[parent_loop->output].base;
    } else {
        auto size = make_shared<Sub>(t_end, t_start);
        auto out_reg = make_shared<AllocRegion>(op.type, size);
        out_sym = out_reg->GetSym(ctx.sym->name);
        ctx.loop->syms[out_sym] = out_reg;
    }

    vector<ExprPtr> args = {t_start, t_end, out_sym};
    for (const auto& input: child_op->inputs) {
        args.push_back(eval(input));
    }
    ctx.val = make_shared<Call>(child_loop, move(args));
}

void LoopGen::Visit(const AggExpr&) {}