#ifndef INCLUDE_TILT_CODEGEN_LLVMGEN_H_
#define INCLUDE_TILT_CODEGEN_LLVMGEN_H_

#include <memory>
#include <utility>
#include <string>
#include <vector>

#include "tilt/codegen/irgen.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"

using namespace std;

namespace tilt {

class LLVMGenCtx : public IRGenCtx<Expr, llvm::Value*> {
public:
    LLVMGenCtx(const Loop* loop, llvm::LLVMContext* llctx) :
        IRGenCtx(nullptr, &loop->syms, new map<Sym, llvm::Value*>()),
        loop(loop), llctx(llctx), map_backup(unique_ptr<map<Sym, llvm::Value*>>(out_sym_tbl))
    {}

private:
    const Loop* loop;
    llvm::LLVMContext* llctx;
    unique_ptr<map<Sym, llvm::Value*>> map_backup;
    friend class LLVMGen;
};

class LLVMGen : public IRGen<LLVMGenCtx, Expr, llvm::Value*> {
public:
    explicit LLVMGen(LLVMGenCtx llgenctx) :
        IRGen(move(llgenctx)), _llctx(*llgenctx.llctx),
        _llmod(make_unique<llvm::Module>(llgenctx.loop->name, _llctx)),
        _builder(make_unique<llvm::IRBuilder<>>(_llctx))
    {
        register_vinstrs();
    }

    static unique_ptr<llvm::Module> Build(const Looper, llvm::LLVMContext&);

private:
    llvm::Value* visit(const Symbol&) final;
    llvm::Value* visit(const IfElse&) final;
    llvm::Value* visit(const Get&) final;
    llvm::Value* visit(const New&) final;
    llvm::Value* visit(const Exists&) final;
    llvm::Value* visit(const ConstNode&) final;
    llvm::Value* visit(const NaryExpr&) final;
    llvm::Value* visit(const SubLStream&) final { throw std::runtime_error("Invalid expression"); }
    llvm::Value* visit(const Element&) final { throw std::runtime_error("Invalid expression"); }
    llvm::Value* visit(const OpNode&) final { throw std::runtime_error("Invalid expression"); }
    llvm::Value* visit(const AggNode&) final { throw std::runtime_error("Invalid expression"); }
    llvm::Value* visit(const AllocIndex&) final;
    llvm::Value* visit(const GetTime&) final;
    llvm::Value* visit(const GetIndex&) final;
    llvm::Value* visit(const Fetch&) final;
    llvm::Value* visit(const Load&) final;
    llvm::Value* visit(const Store&) final;
    llvm::Value* visit(const Advance&) final;
    llvm::Value* visit(const NextTime&) final;
    llvm::Value* visit(const GetStartIdx&) final;
    llvm::Value* visit(const GetEndIdx&) final;
    llvm::Value* visit(const CommitData&) final;
    llvm::Value* visit(const CommitNull&) final;
    llvm::Value* visit(const AllocRegion&) final;
    llvm::Value* visit(const MakeRegion&) final;
    llvm::Value* visit(const Call&) final;
    llvm::Value* visit(const Loop&) final;

    void assign(const Sym& sym_ptr, llvm::Value* val) override
    {
        IRGen::assign(sym_ptr, val);
        val->setName(sym_ptr->name);
    }

    void register_vinstrs();

    llvm::Function* llfunc(const string, llvm::Type*, vector<llvm::Type*>);
    llvm::Value* llcall(const string, llvm::Type*, vector<llvm::Value*>);
    llvm::Value* llcall(const string, llvm::Type*, vector<Expr>);

    llvm::Value* llsizeof(llvm::Type*);

    llvm::Type* lltype(const DataType&);
    llvm::Type* lltype(const Type&);
    llvm::Type* lltype(const ExprNode& expr) { return lltype(expr.type); }
    llvm::Type* lltype(const Expr& expr) { return lltype(expr->type); }

    llvm::Module* llmod() { return _llmod.get(); }
    llvm::LLVMContext& llctx() { return _llctx; }
    llvm::IRBuilder<>* builder() { return _builder.get(); }

    llvm::LLVMContext& _llctx;
    unique_ptr<llvm::Module> _llmod;
    unique_ptr<llvm::IRBuilder<>> _builder;
};

}  // namespace tilt

#endif  // INCLUDE_TILT_CODEGEN_LLVMGEN_H_
