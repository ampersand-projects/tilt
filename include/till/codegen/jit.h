#if 0
#ifndef TILL_JIT
#define TILL_JIT

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

namespace till {

    class JIT
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
        JIT(JITTargetMachineBuilder jtmb, DataLayout dl) :
            linker(es, []() { return std::make_unique<SectionMemoryManager>(); }),
            compiler(es, linker, std::make_unique<ConcurrentIRCompiler>(std::move(jtmb))),
            dl(std::move(dl)), mangle(es, this->dl), ctx(std::make_unique<LLVMContext>()),
            jd(es.createBareJITDylib("__till_lib"))
        {
            jd.addGenerator(cantFail(
                DynamicLibrarySearchGenerator::GetForCurrentProcess(dl.getGlobalPrefix())));
        }

        static std::unique_ptr<JIT> Create()
        {
            auto jtmb = std::move(cantFail(JITTargetMachineBuilder::detectHost()));
            auto dl = std::move(cantFail(jtmb.getDefaultDataLayoutForTarget()));
            return std::make_unique<JIT>(std::move(jtmb), std::move(dl));
        }

        const DataLayout& getDataLayout() const { return dl; }

        LLVMContext& getContext() { return *(ctx.getContext()); }

        void addModule(std::unique_ptr<Module> M)
        {
            cantFail(compiler.add(jd, ThreadSafeModule(std::move(M), ctx)));
        }

        JITEvaluatedSymbol lookup(StringRef name)
        {
            return cantFail(es.lookup({ &jd }, mangle(name.str())));
        }
    }; // class JIT
} // namespace till

#endif // TILL_JIT
#endif
