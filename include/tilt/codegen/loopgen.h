#ifndef INCLUDE_TILT_CODEGEN_LOOPGEN_H_
#define INCLUDE_TILT_CODEGEN_LOOPGEN_H_

#include <stdexcept>

#include "tilt/codegen/irgen.h"

using namespace std;

namespace tilt {

class LoopGenCtx : public IRGenCtx<Expr, Expr> {
public:
    LoopGenCtx(Sym sym, const OpNode* op, Looper loop) :
        IRGenCtx(sym, &op->syms, &loop->syms), op(op), loop(loop)
    {}

private:
    const OpNode* op;
    Looper loop;

    map<Sym, map<Point, Indexer>> pt_idx_maps;

    friend class LoopGen;
};

class LoopGen : public IRGen<LoopGenCtx, Expr, Expr> {
public:
    explicit LoopGen(LoopGenCtx ctx) : IRGen(ctx) {}

    static Looper Build(Sym, const OpNode*);

private:
    Expr get_timer(const Point);
    Indexer& create_idx(const Sym, const Point);
    void build_loop();

    Expr visit(const Symbol&) final;
    Expr visit(const Call&) final;
    Expr visit(const IfElse&) final;
    Expr visit(const Select&) final;
    Expr visit(const Read&) final;
    Expr visit(const Get&) final;
    Expr visit(const New&) final;
    Expr visit(const Exists&) final;
    Expr visit(const ConstNode&) final;
    Expr visit(const Cast&) final;
    Expr visit(const NaryExpr&) final;
    Expr visit(const SubLStream&) final;
    Expr visit(const Element&) final;
    Expr visit(const OpNode&) final;
    Expr visit(const AggNode&) final;
    Expr visit(const Fetch&) final { throw runtime_error("Invalid expression"); };
    Expr visit(const Write&) final { throw runtime_error("Invalid expression"); };
    Expr visit(const Advance&) final { throw runtime_error("Invalid expression"); };
    Expr visit(const GetCkpt&) final { throw runtime_error("Invalid expression"); };
    Expr visit(const GetStartIdx&) final { throw runtime_error("Invalid expression"); };
    Expr visit(const GetEndIdx&) final { throw runtime_error("Invalid expression"); };
    Expr visit(const CommitData&) final { throw runtime_error("Invalid expression"); };
    Expr visit(const CommitNull&) final { throw runtime_error("Invalid expression"); };
    Expr visit(const AllocRegion&) final { throw runtime_error("Invalid expression"); };
    Expr visit(const MakeRegion&) final { throw runtime_error("Invalid expression"); };
    Expr visit(const Loop&) final { throw runtime_error("Invalid expression"); };
};

}  // namespace tilt

#endif  // INCLUDE_TILT_CODEGEN_LOOPGEN_H_
