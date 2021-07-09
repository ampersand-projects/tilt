#ifndef TILT_LOOPGEN
#define TILT_LOOPGEN

#include "tilt/codegen/irgen.h"

#include <stdexcept>

using namespace std;

namespace tilt
{

    class LoopGenCtx : public IRGenCtx<ExprPtr, ExprPtr> {
    public:
        LoopGenCtx(SymPtr sym, const Op* op, Looper loop) :
            IRGenCtx(sym, &op->syms, &loop->syms), op(op), loop(loop)
        {}

    private:
        const Op* op;
        Looper loop;

        map<SymPtr, SymPtr> sym_ref_map;
        map<SymPtr, map<Point, Indexer>> pt_idx_maps;

        friend class LoopGen;
    };

    class LoopGen : public IRGen<LoopGenCtx, ExprPtr, ExprPtr> {
    public:
        LoopGen(LoopGenCtx ctx) : IRGen(ctx) {}

        static Looper Build(SymPtr sym, const Op* op)
        {
            auto loop = make_shared<Loop>(sym);
            LoopGenCtx ctx(sym, op, loop);
            LoopGen loopgen(move(ctx));
            loopgen.build_loop();
            return loopgen.ctx().loop;
        }

    private:
        Indexer& create_idx(const SymPtr, const Point);
        void build_loop();

        SymPtr& sym_ref(const SymPtr& sym) { return ctx().sym_ref_map[sym]; }

        ExprPtr visit(const Symbol&) final;
        ExprPtr visit(const IfElse&) final;
        ExprPtr visit(const Exists&) final;
        ExprPtr visit(const Equals&) final;
        ExprPtr visit(const Not&) final;
        ExprPtr visit(const And&) final;
        ExprPtr visit(const Or&) final;
        ExprPtr visit(const IConst&) final;
        ExprPtr visit(const UConst&) final;
        ExprPtr visit(const FConst&) final;
        ExprPtr visit(const CConst&) final;
        ExprPtr visit(const TConst&) final;
        ExprPtr visit(const Add&) final;
        ExprPtr visit(const Sub&) final;
        ExprPtr visit(const Max&) final;
        ExprPtr visit(const Min&) final;
        ExprPtr visit(const Now&) final;
        ExprPtr visit(const True&) final;
        ExprPtr visit(const False&) final;
        ExprPtr visit(const LessThan&) final;
        ExprPtr visit(const LessThanEqual&) final;
        ExprPtr visit(const GreaterThan&) final;
        ExprPtr visit(const SubLStream&) final;
        ExprPtr visit(const Element&) final;
        ExprPtr visit(const Op&) final;
        ExprPtr visit(const AggExpr&) final;
        ExprPtr visit(const AllocIndex&) final { throw std::runtime_error("Invalid expression"); };
        ExprPtr visit(const GetTime&) final { throw std::runtime_error("Invalid expression"); };
        ExprPtr visit(const GetIndex&) final { throw std::runtime_error("Invalid expression"); };
        ExprPtr visit(const Fetch&) final { throw std::runtime_error("Invalid expression"); };
        ExprPtr visit(const Load&) final { throw std::runtime_error("Invalid expression"); };
        ExprPtr visit(const Store&) final { throw std::runtime_error("Invalid expression"); };
        ExprPtr visit(const Advance&) final { throw std::runtime_error("Invalid expression"); };
        ExprPtr visit(const NextTime&) final { throw std::runtime_error("Invalid expression"); };
        ExprPtr visit(const GetStartIdx&) final { throw std::runtime_error("Invalid expression"); };
        ExprPtr visit(const GetEndIdx&) final { throw std::runtime_error("Invalid expression"); };
        ExprPtr visit(const CommitData&) final { throw std::runtime_error("Invalid expression"); };
        ExprPtr visit(const CommitNull&) final { throw std::runtime_error("Invalid expression"); };
        ExprPtr visit(const AllocRegion&) final { throw std::runtime_error("Invalid expression"); };
        ExprPtr visit(const MakeRegion&) final { throw std::runtime_error("Invalid expression"); };
        ExprPtr visit(const Call&) final { throw std::runtime_error("Invalid expression"); };
        ExprPtr visit(const Loop&) final { throw std::runtime_error("Invalid expression"); };
    };

} // namespace tilt

#endif // TILT_LOOPGEN