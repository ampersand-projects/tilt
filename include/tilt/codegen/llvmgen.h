#ifndef TILT_LLVMGEN
#define TILT_LLVMGEN

#include "tilt/codegen/irgen.h"

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"

#include <memory>

using namespace std;

namespace tilt {

    class LLVMGenCtx : public IRGenCtx<ExprPtr, llvm::Value*> {
    public:
        LLVMGenCtx(Looper loop, map<SymPtr, llvm::Value*>& sym_tbl) :
            IRGenCtx(nullptr, loop->syms, sym_tbl),
            llcontext(move(make_unique<llvm::LLVMContext>())),
            llmodule(move(make_unique<llvm::Module>(loop->name, *llcontext))),
            builder(move(make_unique<llvm::IRBuilder<>>(*llcontext)))
        {}

        unique_ptr<llvm::Module> llmod() { return move(llmodule); }
        unique_ptr<llvm::LLVMContext> llctx() { return move(llcontext); }

    private:
        unique_ptr<llvm::LLVMContext> llcontext;
        unique_ptr<llvm::Module> llmodule;
        unique_ptr<llvm::IRBuilder<>> builder;

        friend class LLVMGen;
    };

    class LLVMGen : public IRGen<LLVMGenCtx, ExprPtr, llvm::Value*> {
    public:
        LLVMGen(LLVMGenCtx ctx) : IRGen(move(ctx)) {}

        static LLVMGenCtx Build(const Looper loop)
        {
            map<SymPtr, llvm::Value*> sym_tbl;
            LLVMGenCtx ctx(loop, sym_tbl);
            LLVMGen llgen(move(ctx));
            loop->Accept(llgen);
            auto llctx = move(llgen.ctx());
            return llctx;
        }

    private:
        llvm::Value* visit(const Symbol&) final;
        llvm::Value* visit(const IfElse&) final;
        llvm::Value* visit(const Exists&) final;
        llvm::Value* visit(const Equals&) final;
        llvm::Value* visit(const Not&) final;
        llvm::Value* visit(const And&) final;
        llvm::Value* visit(const Or&) final;
        llvm::Value* visit(const IConst&) final;
        llvm::Value* visit(const UConst&) final;
        llvm::Value* visit(const FConst&) final;
        llvm::Value* visit(const CConst&) final;
        llvm::Value* visit(const TConst&) final;
        llvm::Value* visit(const Add&) final;
        llvm::Value* visit(const Sub&) final;
        llvm::Value* visit(const Max&) final;
        llvm::Value* visit(const Min&) final;
        llvm::Value* visit(const Now&) final;
        llvm::Value* visit(const True&) final;
        llvm::Value* visit(const False&) final;
        llvm::Value* visit(const LessThan&) final;
        llvm::Value* visit(const LessThanEqual&) final;
        llvm::Value* visit(const GreaterThan&) final;
        llvm::Value* visit(const SubLStream&) { throw std::runtime_error("Invalid expression"); }
        llvm::Value* visit(const Element&) { throw std::runtime_error("Invalid expression"); }
        llvm::Value* visit(const Op&) { throw std::runtime_error("Invalid expression"); }
        llvm::Value* visit(const AggExpr&) final;
        llvm::Value* visit(const AllocIndex&) final;
        llvm::Value* visit(const GetTime&) final;
        llvm::Value* visit(const Fetch&) final;
        llvm::Value* visit(const Load&) final;
        llvm::Value* visit(const Advance&) final;
        llvm::Value* visit(const Next&) final;
        llvm::Value* visit(const GetStartIdx&) final;
        llvm::Value* visit(const CommitData&) final;
        llvm::Value* visit(const CommitNull&) final;
        llvm::Value* visit(const AllocRegion&) final;
        llvm::Value* visit(const MakeRegion&) final;
        llvm::Value* visit(const Call&) final;
        llvm::Value* visit(const Loop&) final;

        llvm::Value*& sym(const SymPtr& sym_ptr)
        {
            map_sym(sym_ptr) = sym_ptr;
            return ctx().out_sym_tbl[sym_ptr];
        }

        llvm::Function* llfunc(const string, llvm::Type*, vector<llvm::Type*>);
        llvm::Value* llcall(const string, llvm::Type*, vector<llvm::Value*>);
        llvm::Value* llcall(const string, llvm::Type*, vector<ExprPtr>);

        llvm::Type* lltype(const PrimitiveType&);
        llvm::Type* lltype(const vector<PrimitiveType>&, const bool);
        llvm::Type* lltype(const DataType& dtype) { return lltype(dtype.ptypes, dtype.is_ptr); }
        llvm::Type* lltype(const Type&);
        llvm::Type* lltype(const Expr& expr) { return lltype(expr.type); }
        llvm::Type* lltype(const ExprPtr& expr) { return lltype(expr->type); }

        llvm::Module* llmod() { return ctx().llmodule.get(); }
        llvm::LLVMContext& llctx() { return *(ctx().llcontext); }
        llvm::IRBuilder<>* builder() { return ctx().builder.get(); }
    };

} // namespace tilt

#endif // TILT_LLVMGEN
