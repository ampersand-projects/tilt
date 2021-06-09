#ifndef TILT_LOOPGEN
#define TILT_LOOPGEN

#include "tilt/codegen/visitor.h"

#include <utility>

using namespace std;

namespace tilt
{

    class LoopGenCtx {
    public:
        LoopGenCtx(SymPtr sym) :
            LoopGenCtx(sym, make_shared<Loop>(sym->name))
        {}

    private:
        LoopGenCtx(SymPtr sym, Looper loop) : sym(sym), loop(loop) {}

        SymPtr sym;
        map<ExprPtr, RegPtr> sym_reg_map;
        map<RegPtr, map<Point, Indexer>> pt_idx_maps;

        Looper loop;

        friend class LoopGen;
    };

    class LoopGen : public Visitor {
    public:
        LoopGen(LoopGenCtx ctx) : ctx(move(ctx)) {}

        Looper result() { return ctx.loop; }

        /**
         * TiLT IR
         */
        void Visit(const Symbol&) final {}
        void Visit(const Lambda&) final {}
        void Visit(const Exists&) final {}
        void Visit(const Equals&) final {}
        void Visit(const Not&) final {}
        void Visit(const And&) final {}
        void Visit(const Or&) final {}
        void Visit(const IConst&) final {}
        void Visit(const UConst&) final {}
        void Visit(const FConst&) final {}
        void Visit(const BConst&) final {}
        void Visit(const CConst&) final {}
        void Visit(const TConst&) final {}
        void Visit(const Add&) final {}
        void Visit(const Sub&) final {}
        void Visit(const Max&) final {}
        void Visit(const Min&) final {}
        void Visit(const Now&) final {}
        void Visit(const True&) final {}
        void Visit(const False&) final {}
        void Visit(const LessThan&) final {}
        void Visit(const LessThanEqual&) final {}

        void Visit(const SubLStream&) override;
        void Visit(const Element&) override;
        void Visit(const Op&) override;
        void Visit(const AggExpr&) override;

        /**
         * Loop IR
         */
        void Visit(const GetTime&) final {}
        void Visit(const Fetch&) final {}
        void Visit(const Advance&) final {}
        void Visit(const Next&) final {}
        void Visit(const CommitData&) final {}
        void Visit(const CommitNull&) final {}
        void Visit(const Block&) final {}
        void Visit(const Loop&) final {}

    private:
        Indexer& create_idx(RegPtr reg, Point pt)
        {
            auto& pt_idx_map = ctx.pt_idx_maps[reg];
            if (pt_idx_map.find(pt) == pt_idx_map.end()) {
                auto idx = make_shared<Index>("i_" + to_string(pt.offset) + "_" + reg->name);
                pt_idx_map[pt] = idx;
                ctx.loop->idx_map[idx] = reg;
            }

            return pt_idx_map[pt];
        }

        void eval(SymPtr sym, ExprPtr expr)
        {
            swap(sym, ctx.sym);
            expr->Accept(*this);
            swap(ctx.sym, sym);
        }

        LoopGenCtx ctx;
    };

} // namespace tilt

#endif // TILT_LOOPGEN