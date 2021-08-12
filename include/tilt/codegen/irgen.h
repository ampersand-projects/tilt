#ifndef INCLUDE_TILT_CODEGEN_IRGEN_H_
#define INCLUDE_TILT_CODEGEN_IRGEN_H_

#include <map>
#include <memory>
#include <utility>

#include "tilt/codegen/visitor.h"
#include "tilt/builder/tilder.h"

using namespace std;

namespace tilt {

template<typename CtxTy, typename InExprTy, typename OutExprTy>
class IRGen;

template<typename InExprTy, typename OutExprTy>
class IRGenCtx {
protected:
    IRGenCtx(Sym sym, const map<Sym, InExprTy>* in_sym_tbl, map<Sym, OutExprTy>* out_sym_tbl) :
        sym(sym), in_sym_tbl(in_sym_tbl), out_sym_tbl(out_sym_tbl)
    {}

    Sym sym;
    const map<Sym, InExprTy>* in_sym_tbl;
    map<Sym, OutExprTy>* out_sym_tbl;
    map<Sym, Sym> sym_map;
    OutExprTy val;

    template<typename CtxTy, typename InTy, typename OutTy>
    friend class IRGen;
};

template<typename CtxTy, typename InExprTy, typename OutExprTy>
class IRGen : public Visitor {
public:
    explicit IRGen(CtxTy ctx) : irctx(move(ctx)) {}

protected:
    virtual OutExprTy visit(const Symbol&) = 0;
    virtual OutExprTy visit(const Out&) = 0;
    virtual OutExprTy visit(const IfElse&) = 0;
    virtual OutExprTy visit(const Select&) = 0;
    virtual OutExprTy visit(const Get&) = 0;
    virtual OutExprTy visit(const New&) = 0;
    virtual OutExprTy visit(const Exists&) = 0;
    virtual OutExprTy visit(const ConstNode&) = 0;
    virtual OutExprTy visit(const Cast&) = 0;
    virtual OutExprTy visit(const NaryExpr&) = 0;
    virtual OutExprTy visit(const SubLStream&) = 0;
    virtual OutExprTy visit(const Element&) = 0;
    virtual OutExprTy visit(const OpNode&) = 0;
    virtual OutExprTy visit(const AggNode&) = 0;
    virtual OutExprTy visit(const Fetch&) = 0;
    virtual OutExprTy visit(const Read&) = 0;
    virtual OutExprTy visit(const Write&) = 0;
    virtual OutExprTy visit(const Advance&) = 0;
    virtual OutExprTy visit(const GetCkpt&) = 0;
    virtual OutExprTy visit(const GetStartIdx&) = 0;
    virtual OutExprTy visit(const GetEndIdx&) = 0;
    virtual OutExprTy visit(const CommitData&) = 0;
    virtual OutExprTy visit(const CommitNull&) = 0;
    virtual OutExprTy visit(const AllocRegion&) = 0;
    virtual OutExprTy visit(const MakeRegion&) = 0;
    virtual OutExprTy visit(const Call&) = 0;
    virtual OutExprTy visit(const Loop&) = 0;

    void Visit(const Out& expr) final { val() = visit(expr); }
    void Visit(const IfElse& expr) final { val() = visit(expr); }
    void Visit(const Select& expr) final { val() = visit(expr); }
    void Visit(const Get& expr) final { val() = visit(expr); }
    void Visit(const New& expr) final { val() = visit(expr); }
    void Visit(const Exists& expr) final { val() = visit(expr); }
    void Visit(const ConstNode& expr) final { val() = visit(expr); }
    void Visit(const Cast& expr) final { val() = visit(expr); }
    void Visit(const NaryExpr& expr) final { val() = visit(expr); }
    void Visit(const SubLStream& expr) final { val() = visit(expr); }
    void Visit(const Element& expr) final { val() = visit(expr); }
    void Visit(const OpNode& expr) final { val() = visit(expr); }
    void Visit(const AggNode& expr) final { val() = visit(expr); }
    void Visit(const Fetch& expr) final { val() = visit(expr); }
    void Visit(const Read& expr) final { val() = visit(expr); }
    void Visit(const Write& expr) final { val() = visit(expr); }
    void Visit(const Advance& expr) final { val() = visit(expr); }
    void Visit(const GetCkpt& expr) final { val() = visit(expr); }
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

    Sym tmp_sym(const Symbol& symbol)
    {
        shared_ptr<Symbol> tmp_sym(const_cast<Symbol*>(&symbol), [](Symbol*) {});
        return tmp_sym;
    }

    OutExprTy get_expr(const Sym& sym) { auto& m = *(ctx().out_sym_tbl); return m.at(sym); }
    OutExprTy get_expr(const Symbol& symbol) { return get_expr(tmp_sym(symbol)); }

    virtual void set_expr(const Sym& sym, OutExprTy val)
    {
        set_sym(sym, sym);
        auto& m = *(ctx().out_sym_tbl);
        m[sym] = val;
    }
    void set_expr(const Symbol& symbol, OutExprTy val) { set_expr(tmp_sym(symbol), val); }

    Sym& get_sym(const Sym& in_sym) { return ctx().sym_map.at(in_sym); }
    Sym& get_sym(const Symbol& symbol) { return get_sym(tmp_sym(symbol)); }
    void set_sym(const Sym& in_sym, const Sym out_sym) { ctx().sym_map[in_sym] = out_sym; }
    void set_sym(const Symbol& in_symbol, const Sym out_sym) { set_sym(tmp_sym(in_symbol), out_sym); }

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
        auto tmp = tmp_sym(symbol);

        if (ctx().sym_map.find(tmp) == ctx().sym_map.end()) {
            auto expr = ctx().in_sym_tbl->at(tmp);

            swap(ctx().sym, tmp);
            auto value = eval(expr);
            swap(tmp, ctx().sym);

            auto sym_clone = tilder::_sym(symbol);
            set_sym(tmp, sym_clone);
            this->set_expr(sym_clone, value);
        }

        val() = visit(symbol);
    }

private:
    CtxTy irctx;
};

}  // namespace tilt

#endif  // INCLUDE_TILT_CODEGEN_IRGEN_H_
