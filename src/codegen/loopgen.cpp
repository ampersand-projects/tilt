#include "tilt/codegen/loopgen.h"

#include <string>
#include <unordered_set>

using namespace tilt;
using namespace std;

void LoopGen::Visit(const SubLStream&) {}

void LoopGen::Visit(const Element& elem)
{
    auto& reg = ctx.sym_reg_map[elem.lstream];
    auto& idx = create_idx(reg, elem.pt);
    loop->vars[ctx.sym] = make_shared<Fetch>(reg, idx);
}

void LoopGen::Visit(const Op& op)
{
    auto name = ctx.sym->name;
    loop = make_shared<Loop>(name);

    loop->t_start = make_shared<Time>("t_start_" + name);
    loop->t_end = make_shared<Time>("t_end_" + name);

    for (auto& in : op.inputs) {
        auto in_reg = make_shared<Region>("reg_in_" + in->name, in->type);
        loop->in_regs.push_back(in_reg);
        ctx.sym_reg_map[in] = in_reg;
    }

    loop->out_reg = make_shared<Region>("reg_out_" + name, op.type);

    loop->t_cur = make_shared<Time>("t_cur_" + name);
    loop->t_prev = make_shared<Time>("t_prev_" + name);

    for (auto& var: op.vars) {
        eval(var, op.syms.at(var));
    }

    unordered_set<Indexer> idx_bounds;
    for (const auto& [reg, pt_idx_map]: ctx.pt_idx_maps) {
        auto& tail_idx = pt_idx_map.begin()->second;
        auto& head_idx = pt_idx_map.rbegin()->second;
        idx_bounds.insert(tail_idx);
        idx_bounds.insert(head_idx);

        auto base_idx = tail_idx;
        for (const auto& [pt, idx]: pt_idx_map) {
            auto offset = make_shared<TConst>(pt.offset);
            auto time_expr = make_shared<Add>(loop->t_cur, offset);
            auto adv_expr = make_shared<Advance>(reg, base_idx, time_expr);
            loop->idx_update[idx] = adv_expr;
            base_idx = idx;
        }
    }

    ExprPtr delta = make_shared<TConst>(0);
    for (const auto& idx: idx_bounds) {
        auto next_idx_expr = make_shared<Next>(loop->idx_map[idx], idx);
        auto next_time_expr = make_shared<GetTime>(next_idx_expr);
        auto cur_time_expr = make_shared<GetTime>(idx);
        auto diff_expr = make_shared<Sub>(next_time_expr, cur_time_expr);
        delta = make_shared<Min>(delta, diff_expr);
    }

    auto t_incr = make_shared<Max>(make_shared<TConst>(op.iter.period), delta);
    loop->next_t = make_shared<Add>(loop->t_cur, t_incr);

    loop->pred = op.pred;
    auto data = op.syms.find(op.output)->second;
    loop->true_body = make_shared<CommitData>(loop->out_reg, loop->t_cur, data);
    loop->false_body = make_shared<CommitNull>(loop->out_reg, loop->t_cur);
}

void LoopGen::Visit(const AggExpr&) {}