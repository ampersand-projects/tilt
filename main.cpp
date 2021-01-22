#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/SourceMgr.h"

#include "till/jit.h"

#include <iostream>
#include <memory>
#include <string>

using namespace llvm;
using namespace llvm::orc;

std::unique_ptr<Module> buildprog(LLVMContext& ctx)
{
    std::unique_ptr<Module> llmod = std::make_unique<Module>("test", ctx);

    Module* m = llmod.get();

    Function* add1_fn = Function::Create(
        FunctionType::get(Type::getInt32Ty(ctx), { Type::getInt32Ty(ctx) }, false),
        Function::ExternalLinkage, "add1", m);

    BasicBlock* bb = BasicBlock::Create(ctx, "EntryBlock", add1_fn);

    IRBuilder<> builder(bb);

    Value* one = builder.getInt32(1);

    Argument* argx = add1_fn->getArg(0);
    argx->setName("x");

    Value* add = builder.CreateAdd(one, argx);

    builder.CreateRet(add);

    return llmod;
}

std::unique_ptr<Module> buildsrc(LLVMContext& Context)
{
    const StringRef add1_src =
        R"(
        define i32 @add1(i32 %x) {
        entry:
            %r = add nsw i32 %x, 1
            ret i32 %r
        }
    )";

    SMDiagnostic err;
    auto llmod = parseIR(MemoryBufferRef(add1_src, "test"), err, Context);
    if (!llmod) {
        std::cout << err.getMessage().str() << std::endl;
        return nullptr;
    }
    else {
        return llmod;
    }
}

int main()
{
    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();

    auto jit = cantFail(till::JIT::Create());
    auto& ctx = jit->getContext();

    auto llmod = buildprog(ctx);
    //auto llmod = std::move(buildsrc(ctx));
    if (!llmod) return -1;

    cantFail(jit->addModule(std::move(llmod)));

    JITEvaluatedSymbol sym = cantFail(jit->lookup("add1"));

    auto* add1 = (int (*)(int))(intptr_t)sym.getAddress();
    std::cout << "Result: " << add1(10) << std::endl;

    return 0;
}