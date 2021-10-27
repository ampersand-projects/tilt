#ifndef TILT_INCLUDE_TILT_CODEGEN_LOOPGEN_H_
#define TILT_INCLUDE_TILT_CODEGEN_LOOPGEN_H_

#include <stdexcept>
#include <utility>

#include "tilt/pass/irgen.h"

using namespace std;

namespace tilt {

class LoopGenCtx : public IRGenCtx<Expr, Expr> {
public:
    LoopGenCtx(Sym sym, const OpNode* op, Loop loop) :
        IRGenCtx(sym, &op->syms, &loop->syms), op(op), loop(loop)
    {}

private:
    const OpNode* op;
    Loop loop;

    map<Sym, map<Point, Index>> pt_idx_maps;
    map<Index, Expr> idx_diff_map;
    map<Sym, Sym> sym_ref;

    friend class LoopGen;
};

class LoopGen : public IRGen<LoopGenCtx, Expr, Expr> {
public:
    explicit LoopGen(LoopGenCtx ctx) : _ctx(move(ctx)) {}

    static Loop Build(Sym, const OpNode*);

private:
    LoopGenCtx& ctx() override { return _ctx; }

    Expr get_timer(const Point, bool);
    Index& get_idx(const Sym, const Point);
    Sym get_ref(const Sym sym) { return ctx().sym_ref.at(sym); }
    void set_ref(Sym sym, Sym ref) { ctx().sym_ref[sym] = ref; }
    void build_tloop(function<Expr()>, function<Expr()>);
    void build_loop();

    Expr visit(const Symbol&) final;
    Expr visit(const Out&) final;
    Expr visit(const Beat&) final;
    Expr visit(const Call&) final;
    Expr visit(const IfElse&) final;
    Expr visit(const Select&) final;
    Expr visit(const Get&) final;
    Expr visit(const New&) final;
    Expr visit(const Exists&) final;
    Expr visit(const ConstNode&) final;
    Expr visit(const Cast&) final;
    Expr visit(const NaryExpr&) final;
    Expr visit(const SubLStream&) final;
    Expr visit(const Element&) final;
    Expr visit(const OpNode&) final;
    Expr visit(const Reduce&) final;
    Expr visit(const Fetch&) final { throw runtime_error("Invalid expression"); };
    Expr visit(const Read&) final { throw runtime_error("Invalid expression"); };
    Expr visit(const Write&) final { throw runtime_error("Invalid expression"); };
    Expr visit(const Advance&) final { throw runtime_error("Invalid expression"); };
    Expr visit(const GetCkpt&) final { throw runtime_error("Invalid expression"); };
    Expr visit(const GetStartIdx&) final { throw runtime_error("Invalid expression"); };
    Expr visit(const GetEndIdx&) final { throw runtime_error("Invalid expression"); };
    Expr visit(const GetStartTime&) final { throw runtime_error("Invalid expression"); };
    Expr visit(const GetEndTime&) final { throw runtime_error("Invalid expression"); };
    Expr visit(const CommitData&) final { throw runtime_error("Invalid expression"); };
    Expr visit(const CommitNull&) final { throw runtime_error("Invalid expression"); };
    Expr visit(const AllocRegion&) final { throw runtime_error("Invalid expression"); };
    Expr visit(const MakeRegion&) final { throw runtime_error("Invalid expression"); };
    Expr visit(const LoopNode&) final { throw runtime_error("Invalid expression"); };

    LoopGenCtx _ctx;
};

}  // namespace tilt

#endif  // TILT_INCLUDE_TILT_CODEGEN_LOOPGEN_H_
