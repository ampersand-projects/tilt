#ifndef INCLUDE_TILT_ENGINE_ENGINE_H_
#define INCLUDE_TILT_ENGINE_ENGINE_H_

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
        _llmod(make_unique<Module>("vinst", GetCtx())),
        jd(es.createBareJITDylib("__tilt_dylib"))
    {
        jd.addGenerator(cantFail(DynamicLibrarySearchGenerator::GetForCurrentProcess(dl.getGlobalPrefix())));
        register_struct_types();
    }

    static ExecEngine* Get();
    void AddModule(unique_ptr<Module>);
    LLVMContext& GetCtx();
    Module* llmod() { return _llmod.get(); }
    intptr_t Lookup(StringRef);

private:
    void register_struct_types();
    
    static Expected<ThreadSafeModule> optimize_module(ThreadSafeModule, const MaterializationResponsibility&);

    ExecutionSession es;
    RTDyldObjectLinkingLayer linker;
    IRCompileLayer compiler;
    IRTransformLayer optimizer;

    DataLayout dl;
    MangleAndInterner mangler;
    ThreadSafeContext ctx;
    unique_ptr<Module> _llmod;

    JITDylib& jd;
};

}  // namespace tilt

#endif  // INCLUDE_TILT_ENGINE_ENGINE_H_
