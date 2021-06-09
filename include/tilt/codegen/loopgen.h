#ifndef TILT_LOOPGEN
#define TILT_LOOPGEN

#include "tilt/codegen/visitor.h"

#include <utility>

using namespace std;

namespace tilt
{

    class LoopGenCtx {
        LoopGenCtx(SymPtr sym, const Op* op, Looper loop) :
            sym(sym), op(op), loop(loop)
        {}

        SymPtr sym;
        const Op* op;
        Looper loop;

        ExprPtr val;

        map<SymPtr, SymPtr> sym_sym_map;
        map<SymPtr, SymPtr> sym_ref_map;
        map<SymPtr, map<Point, Indexer>> pt_idx_maps;

        friend class LoopGen;
    };

    class LoopGen : public Visitor {
    public:
        static Looper Build(SymPtr sym, const Op* op);

    private:
        LoopGen(LoopGenCtx ctx) : ctx(ctx) {}

        void build_loop();

        Indexer& create_idx(const SymPtr, const Point);

        ExprPtr eval(const ExprPtr expr)
        {
            ExprPtr val = nullptr;

            swap(val, ctx.val);
            expr->Accept(*this);
            swap(ctx.val, val);

            return val;
        }

        /**
         * TiLT IR
         */
        void Visit(const Symbol&) override;
        void Visit(const IfElse&) override;
        void Visit(const Exists&) override;
        void Visit(const Equals&) override;
        void Visit(const Not&) override;
        void Visit(const And&) override;
        void Visit(const Or&) override;
        void Visit(const IConst&) override;
        void Visit(const UConst&) override;
        void Visit(const FConst&) override;
        void Visit(const BConst&) override;
        void Visit(const CConst&) override;
        void Visit(const TConst&) override;
        void Visit(const Add&) override;
        void Visit(const Sub&) override;
        void Visit(const Max&) override;
        void Visit(const Min&) override;
        void Visit(const Now&) override;
        void Visit(const True&) override;
        void Visit(const False&) override;
        void Visit(const LessThan&) override;
        void Visit(const LessThanEqual&) override;
        void Visit(const GreaterThan&) override;

        void Visit(const SubLStream&) override;
        void Visit(const Element&) override;
        void Visit(const Op&) override;
        void Visit(const AggExpr&) override;

        /**
         * Loop IR
         */
        void Visit(const AllocIndex&) final {}
        void Visit(const GetTime&) final {}
        void Visit(const Fetch&) final {}
        void Visit(const Load&) final {}
        void Visit(const Advance&) final {}
        void Visit(const Next&) final {}
        void Visit(const GetStartIdx&) final {}
        void Visit(const CommitData&) final {}
        void Visit(const CommitNull&) final {}
        void Visit(const AllocRegion&) final {}
        void Visit(const MakeRegion&) final {}
        void Visit(const Call&) final {}
        void Visit(const Loop&) final {}

        LoopGenCtx ctx;
    };

} // namespace tilt

#endif // TILT_LOOPGEN