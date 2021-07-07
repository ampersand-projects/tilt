#ifndef TILT_IRGEN
#define TILT_IRGEN

#include "tilt/codegen/visitor.h"

#include <map>

using namespace std;

namespace tilt
{
    template<typename CtxTy, typename InExprTy, typename OutExprTy>
    class IRGen;

    template<typename InExprTy, typename OutExprTy>
    class IRGenCtx {
    protected:
        IRGenCtx(SymPtr sym, const map<SymPtr, InExprTy>* in_sym_tbl, map<SymPtr, OutExprTy>* out_sym_tbl) :
            sym(sym), in_sym_tbl(in_sym_tbl), out_sym_tbl(out_sym_tbl)
        {}

        SymPtr sym;
        const map<SymPtr, InExprTy>* in_sym_tbl;
        map<SymPtr, OutExprTy>* out_sym_tbl;
        map<SymPtr, SymPtr> sym_map;
        OutExprTy val;

        template<typename CtxTy, typename InTy, typename OutTy>
        friend class IRGen;
    };

    template<typename CtxTy, typename InExprTy, typename OutExprTy>
    class IRGen : public Visitor {
    public:
        IRGen(CtxTy ctx) : irctx(move(ctx)) {}

    protected:
        virtual OutExprTy visit(const Symbol&) = 0;
        virtual OutExprTy visit(const IfElse&) = 0;
        virtual OutExprTy visit(const Exists&) = 0;
        virtual OutExprTy visit(const Equals&) = 0;
        virtual OutExprTy visit(const Not&) = 0;
        virtual OutExprTy visit(const And&) = 0;
        virtual OutExprTy visit(const Or&) = 0;
        virtual OutExprTy visit(const IConst&) = 0;
        virtual OutExprTy visit(const UConst&) = 0;
        virtual OutExprTy visit(const FConst&) = 0;
        virtual OutExprTy visit(const CConst&) = 0;
        virtual OutExprTy visit(const TConst&) = 0;
        virtual OutExprTy visit(const Add&) = 0;
        virtual OutExprTy visit(const Sub&) = 0;
        virtual OutExprTy visit(const Max&) = 0;
        virtual OutExprTy visit(const Min&) = 0;
        virtual OutExprTy visit(const Now&) = 0;
        virtual OutExprTy visit(const True&) = 0;
        virtual OutExprTy visit(const False&) = 0;
        virtual OutExprTy visit(const LessThan&) = 0;
        virtual OutExprTy visit(const LessThanEqual&) = 0;
        virtual OutExprTy visit(const GreaterThan&) = 0;
        virtual OutExprTy visit(const SubLStream&) = 0;
        virtual OutExprTy visit(const Element&) = 0;
        virtual OutExprTy visit(const Op&) = 0;
        virtual OutExprTy visit(const AggExpr&) = 0;
        virtual OutExprTy visit(const AllocIndex&) = 0;
        virtual OutExprTy visit(const GetTime&) = 0;
        virtual OutExprTy visit(const Fetch&) = 0;
        virtual OutExprTy visit(const Load&) = 0;
        virtual OutExprTy visit(const Store&) = 0;
        virtual OutExprTy visit(const Advance&) = 0;
        virtual OutExprTy visit(const NextTime&) = 0;
        virtual OutExprTy visit(const GetStartIdx&) = 0;
        virtual OutExprTy visit(const GetEndIdx&) = 0;
        virtual OutExprTy visit(const CommitData&) = 0;
        virtual OutExprTy visit(const CommitNull&) = 0;
        virtual OutExprTy visit(const AllocRegion&) = 0;
        virtual OutExprTy visit(const MakeRegion&) = 0;
        virtual OutExprTy visit(const Call&) = 0;
        virtual OutExprTy visit(const Loop&) = 0;

        void Visit(const IfElse& expr) final { val() = visit(expr); }
        void Visit(const Exists& expr) final { val() = visit(expr); }
        void Visit(const Equals& expr) final { val() = visit(expr); }
        void Visit(const Not& expr) final { val() = visit(expr); }
        void Visit(const And& expr) final { val() = visit(expr); }
        void Visit(const Or& expr) final { val() = visit(expr); }
        void Visit(const IConst& expr) final { val() = visit(expr); }
        void Visit(const UConst& expr) final { val() = visit(expr); }
        void Visit(const FConst& expr) final { val() = visit(expr); }
        void Visit(const CConst& expr) final { val() = visit(expr); }
        void Visit(const TConst& expr) final { val() = visit(expr); }
        void Visit(const Add& expr) final { val() = visit(expr); }
        void Visit(const Sub& expr) final { val() = visit(expr); }
        void Visit(const Max& expr) final { val() = visit(expr); }
        void Visit(const Min& expr) final { val() = visit(expr); }
        void Visit(const Now& expr) final { val() = visit(expr); }
        void Visit(const True& expr) final { val() = visit(expr); }
        void Visit(const False& expr) final { val() = visit(expr); }
        void Visit(const LessThan& expr) final { val() = visit(expr); }
        void Visit(const LessThanEqual& expr) final { val() = visit(expr); }
        void Visit(const GreaterThan& expr) final { val() = visit(expr); }
        void Visit(const SubLStream& expr) final { val() = visit(expr); }
        void Visit(const Element& expr) final { val() = visit(expr); }
        void Visit(const Op& expr) final { val() = visit(expr); }
        void Visit(const AggExpr& expr) final { val() = visit(expr); }
        void Visit(const AllocIndex& expr) final { val() = visit(expr); }
        void Visit(const GetTime& expr) final { val() = visit(expr); }
        void Visit(const Fetch& expr) final { val() = visit(expr); }
        void Visit(const Load& expr) final { val() = visit(expr); }
        void Visit(const Store& expr) final { val() = visit(expr); }
        void Visit(const Advance& expr) final { val() = visit(expr); }
        void Visit(const NextTime& expr) final { val() = visit(expr); }
        void Visit(const GetStartIdx& expr) final { val() = visit(expr); }
        void Visit(const GetEndIdx& expr) final { val() = visit(expr); }
        void Visit(const CommitData& expr) final { val() = visit(expr); }
        void Visit(const CommitNull& expr) final { val() = visit(expr); }
        void Visit(const AllocRegion& expr) final { val() = visit(expr); }
        void Visit(const MakeRegion& expr) final { val() = visit(expr); }
        void Visit(const Call& expr) final { val() = visit(expr); }
        void Visit(const Loop& expr) final { val() = visit(expr); }

        CtxTy& ctx() { return irctx; }

        CtxTy& switch_ctx(CtxTy& new_ctx) { swap(new_ctx, irctx); return new_ctx; }

        OutExprTy& sym(const SymPtr& sym_ptr) { auto& m = *(ctx().out_sym_tbl); return m[sym_ptr]; }

        SymPtr& map_sym(const SymPtr& in_sym) { return ctx().sym_map[in_sym]; }

        SymPtr get_sym(const Symbol& symbol)
        {
            shared_ptr<Symbol> tmp_sym(const_cast<Symbol*>(&symbol), [](Symbol*) {});
            return tmp_sym;
        }

        OutExprTy& val() { return ctx().val; }

        OutExprTy eval(const InExprTy expr)
        {
            OutExprTy val = nullptr;

            swap(val, ctx().val);
            expr->Accept(*this);
            swap(ctx().val, val);

            return val;
        }

        void Visit(const Symbol& symbol) final
        {
            auto tmp_sym = get_sym(symbol);

            if (ctx().sym_map.find(tmp_sym) == ctx().sym_map.end()) {
                auto expr = ctx().in_sym_tbl->at(tmp_sym);

                swap(ctx().sym, tmp_sym);
                auto value = eval(expr);
                swap(tmp_sym, ctx().sym);

                auto sym_clone = make_shared<Symbol>(symbol);
                map_sym(tmp_sym) = sym_clone;
                sym(sym_clone) = value;
            }

            val() = visit(symbol);
        }

    private:
        CtxTy irctx;
    };

} // namespace tilt

#endif // TILT_IRGEN