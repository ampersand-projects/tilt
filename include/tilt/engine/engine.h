#ifndef TILT_ENGINE
#define TILT_ENGINE

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

#include "easy/jit.h"

#include <memory>

using namespace std;
using namespace llvm;
using namespace llvm::orc;

index_t* get_start_idx(region_t* reg);
long get_time(index_t* idx);
long next_time(region_t* reg, index_t* idx);
index_t* advance(region_t* reg, index_t* idx, long t);
char* fetch(region_t* reg, index_t* idx, size_t size);
index_t* commit_data(region_t* reg, long t);
index_t* commit_null(region_t* reg, long t);

namespace tilt {

    struct index_t {
        long t;
        unsigned int i;
    };

    struct region_t {
        index_t si;
        index_t ei;
        index_t* tl;
        char* data;
    };

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
            jd.addGenerator(
                cantFail(DynamicLibrarySearchGenerator::GetForCurrentProcess(dl.getGlobalPrefix()))
            );

            register_symbols();
        }

        static ExecEngine* Get()
        {
            static unique_ptr<ExecEngine> engine;

            if (!engine) {
                InitializeNativeTarget();
                InitializeNativeTargetAsmPrinter();

                auto jtmb = cantFail(JITTargetMachineBuilder::detectHost());
                auto dl = cantFail(jtmb.getDefaultDataLayoutForTarget());

                engine = make_unique<ExecEngine>(move(jtmb), move(dl));
            }

            return engine.get();
        }

        void AddModule(unique_ptr<Module> m)
        {
            cantFail(optimizer.add(jd, ThreadSafeModule(move(m), ctx)));
        }

        LLVMContext& GetCtx() { return *ctx.getContext(); }

        intptr_t Lookup(StringRef name)
        {
            auto fn_sym = cantFail(es.lookup({ &jd }, mangler(name.str())));
            return (intptr_t) fn_sym.getAddress();
        }

    private:
        void register_symbols();

        static Expected<ThreadSafeModule> optimize_module(ThreadSafeModule tsm, const MaterializationResponsibility &r)
        {
            tsm.withModuleDo([](Module &m) {
                unsigned opt_level = 3;
                unsigned opt_size = 0;

                llvm::PassManagerBuilder builder;
                builder.OptLevel = opt_level;
                builder.Inliner = createFunctionInliningPass(opt_level, opt_size, false);

                llvm::legacy::PassManager mpm;
                builder.populateModulePassManager(mpm);
                mpm.run(m);
            });

            return move(tsm);
        }

        ExecutionSession es;
        RTDyldObjectLinkingLayer linker;
        IRCompileLayer compiler;
        IRTransformLayer optimizer;

        DataLayout dl;
        MangleAndInterner mangler;
        ThreadSafeContext ctx;

        JITDylib& jd;
    };

} // namespace tilt

#endif // TILT_ENGINE