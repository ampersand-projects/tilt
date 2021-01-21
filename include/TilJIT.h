#ifndef TIL_TILJIT
#define TIL_TILJIT

#include "llvm/ADT/StringRef.h"
#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/ExecutionEngine/Orc/Core.h"
#include "llvm/ExecutionEngine/Orc/ExecutionUtils.h"
#include "llvm/ExecutionEngine/Orc/IRCompileLayer.h"
#include "llvm/ExecutionEngine/Orc/JITTargetMachineBuilder.h"
#include "llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/LLVMContext.h"

#include <memory>

using namespace llvm;
using namespace llvm::orc;

class TilJIT
{
private:
    ExecutionSession es;
    RTDyldObjectLinkingLayer linker;
    IRCompileLayer compiler;

    DataLayout dl;
    MangleAndInterner mangle;
    ThreadSafeContext ctx;

    JITDylib& jd;

public:
    TilJIT(JITTargetMachineBuilder jtmb, DataLayout dl) :
        linker(es, []() { return std::make_unique<SectionMemoryManager>(); }),
        compiler(es, linker, std::make_unique<ConcurrentIRCompiler>(std::move(jtmb))),
        dl(std::move(dl)), mangle(es, this->dl), ctx(std::make_unique<LLVMContext>()),
        jd(es.createBareJITDylib("__til_lib"))
    {
        jd.addGenerator(cantFail(
            DynamicLibrarySearchGenerator::GetForCurrentProcess(dl.getGlobalPrefix())));
    }

    static Expected<std::unique_ptr<TilJIT>> Create()
    {
        auto jtmb = JITTargetMachineBuilder::detectHost();
        if (!jtmb) return jtmb.takeError();

        auto dl = jtmb->getDefaultDataLayoutForTarget();
        if (!dl) return dl.takeError();

        return std::make_unique<TilJIT>(std::move(*jtmb), std::move(*dl));
    }

    const DataLayout& getDataLayout() const { return dl; }

    LLVMContext& getContext() { return *(ctx.getContext()); }

    Error addModule(std::unique_ptr<Module> M)
    {
        return compiler.add(jd, ThreadSafeModule(std::move(M), ctx));
    }

    Expected<JITEvaluatedSymbol> lookup(StringRef name)
    {
        return es.lookup({ &jd }, mangle(name.str()));
    }
};

#endif // TIL_TILJIT