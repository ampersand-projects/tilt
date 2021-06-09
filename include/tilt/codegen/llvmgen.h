#ifndef TILT_LLVMGEN
#define TILT_LLVMGEN

#include "tilt/codegen/visitor.h"

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"

#include <memory>

using namespace std;

namespace tilt {

    class LLVMGenCtx {
    public:
        LLVMGenCtx() :
            llctx(move(make_unique<llvm::LLVMContext>())),
            llmod(nullptr), builder(nullptr), val(nullptr)
        {}
    private:
        unique_ptr<llvm::LLVMContext> llctx;
        unique_ptr<llvm::Module> llmod;
        unique_ptr<llvm::IRBuilder<>> builder;
        llvm::Value* val;
        map<Symbol*, llvm::Value*> sym_tbl;
        
        friend class LLVMGen;

        llvm::Module* llmodule() { return llmod.get(); }
        llvm::LLVMContext& llcontext() { return *llctx; }
    };

    class LLVMGen : public Visitor {
    public:
        LLVMGen(LLVMGenCtx ctx) : ctx(move(ctx)) {}

        llvm::Module* result() { return ctx.llmodule(); }

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

        void Visit(const SubLStream&) final {}
        void Visit(const Element&) final {}

        void Visit(const Op&) final {}
        void Visit(const AggExpr&) override;

        /**
         * Loop IR
         */
        void Visit(const AllocIndex&) override;
        void Visit(const GetTime&) override;
        void Visit(const Fetch&) override;
        void Visit(const Load&) override;
        void Visit(const Advance&) override;
        void Visit(const Next&) override;
        void Visit(const GetStartIdx&) override;
        void Visit(const CommitData&) override;
        void Visit(const CommitNull&) override;
        void Visit(const AllocRegion&) override;
        void Visit(const MakeRegion&) override;
        void Visit(const Call&) override;
        void Visit(const Loop&) override;

    private:
        llvm::Value* eval(const ExprPtr& expr)
        {
            llvm::Value* val = nullptr;

            swap(val, ctx.val);
            expr->Accept(*this);
            swap(ctx.val, val);

            return val;
        }

        llvm::Value*& sym(const SymPtr& sym)
        {
            return ctx.sym_tbl[sym.get()];
        }

        llvm::Function* get_func(const string, llvm::Type*, vector<llvm::Type*>);

        llvm::Type* lltype(const PrimitiveType&);
        llvm::Type* lltype(const vector<PrimitiveType>&, const bool);
        llvm::Type* lltype(const DataType&);
        llvm::Type* lltype(const Type&);

        LLVMGenCtx ctx;
    };

} // namespace tilt

#endif // TILT_LLVMGEN
