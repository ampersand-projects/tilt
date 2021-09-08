#ifndef TILT_INCLUDE_TILT_ENGINE_ENGINE_H_
#define TILT_INCLUDE_TILT_ENGINE_ENGINE_H_

#include <memory>
#include <utility>

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/ExecutionEngine/Orc/Core.h"
#include "llvm/ExecutionEngine/Orc/ExecutionUtils.h"
#include "llvm/ExecutionEngine/Orc/IRCompileLayer.h"
#include "llvm/ExecutionEngine/Orc/JITTargetMachineBuilder.h"
#include "llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/ExecutionEngine/JITSymbol.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/ExecutionEngine/Orc/IRTransformLayer.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/IPO.h"

using namespace std;
using namespace llvm;
using namespace llvm::orc;

namespace tilt {

class ExecEngine {
public:
    ExecEngine(JITTargetMachineBuilder jtmb, DataLayout dl) :
        linker(es, []() { return make_unique<SectionMemoryManager>(); }),
        compiler(es, linker, make_unique<ConcurrentIRCompiler>(move(jtmb))),
        optimizer(es, compiler, optimize_module),
        dl(move(dl)), mangler(es, this->dl),
        ctx(make_unique<LLVMContext>()),
        jd(es.createBareJITDylib("__tilt_dylib"))
    {
        jd.addGenerator(cantFail(DynamicLibrarySearchGenerator::GetForCurrentProcess(dl.getGlobalPrefix())));
    }

    static ExecEngine* Get();
    void AddModule(unique_ptr<Module>);
    LLVMContext& GetCtx();
    intptr_t Lookup(StringRef);

private:
    static Expected<ThreadSafeModule> optimize_module(ThreadSafeModule, const MaterializationResponsibility&);

    ExecutionSession es;
    RTDyldObjectLinkingLayer linker;
    IRCompileLayer compiler;
    IRTransformLayer optimizer;

    DataLayout dl;
    MangleAndInterner mangler;
    ThreadSafeContext ctx;

    JITDylib& jd;
};

}  // namespace tilt

#endif  // TILT_INCLUDE_TILT_ENGINE_ENGINE_H_
