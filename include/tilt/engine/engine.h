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
#include "llvm/IRReader/IRReader.h"

#include <memory>
#include <iostream>

using namespace std;
using namespace llvm;
using namespace llvm::orc;

namespace tilt {

    struct index_t {
        long t;
        int i;
    };

    struct region_t {
        index_t si;
        index_t ei;
        index_t* tl;
        char* data;
    };

    long get_time(index_t* idx)
    {
        return idx->t;
    }

    long next_time(region_t* reg, index_t* idx)
    {
        return idx->t + 1;
    }

    index_t* advance(region_t* reg, index_t* idx, long t)
    {
        idx->i++;
        idx->t++;
        return idx;
    }

    char* fetch(region_t* reg, index_t* idx, long size)
    {
        return reg->data + (idx->i * size);
    }

    region_t* commit_data(region_t* reg, long t, char* data, long size)
    {
        int i = reg->ei.i;
        char* dptr = reg->data + (i * size);
        for (int k=0; k<size; k++) {
            dptr[k] = data[k];
        }
        reg->ei.i = i+1;
        reg->ei.t = t;
        reg->tl[i].i = 1;
        reg->tl[i].t = t;

        return reg;
    }

    region_t* commit_null(region_t* reg, long t)
    {
        return nullptr;
    }

    class ExecEngine {
    public:
        ExecEngine(JITTargetMachineBuilder jtmb, DataLayout dl) :
            linker(es, []() { return std::make_unique<SectionMemoryManager>(); }),
            compiler(es, linker, std::make_unique<ConcurrentIRCompiler>(std::move(jtmb))),
            optimizer(es, compiler, optimizeModule),
            dl(std::move(dl)), mangler(es, this->dl),
            ctx(std::make_unique<LLVMContext>()),
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

        Error addModule(std::unique_ptr<Module> m)
        {
            return optimizer.add(jd, ThreadSafeModule(move(m), ctx));
        }

        Expected<JITEvaluatedSymbol> lookup(StringRef name)
        {
            return es.lookup({ &jd }, mangler(name.str()));
        }

    private:
        void register_symbols()
        {
            SymbolMap symbols;

            symbols[this->mangler("get_time")] =
                JITEvaluatedSymbol(pointerToJITTargetAddress(&get_time), JITSymbolFlags());
            symbols[this->mangler("next_time")] =
                JITEvaluatedSymbol(pointerToJITTargetAddress(&next_time), JITSymbolFlags());
            symbols[this->mangler("advance")] =
                JITEvaluatedSymbol(pointerToJITTargetAddress(&advance), JITSymbolFlags());
            symbols[this->mangler("fetch")] =
                JITEvaluatedSymbol(pointerToJITTargetAddress(&fetch), JITSymbolFlags());
            symbols[this->mangler("commit_data")] =
                JITEvaluatedSymbol(pointerToJITTargetAddress(&commit_data), JITSymbolFlags());
            symbols[this->mangler("commit_null")] =
                JITEvaluatedSymbol(pointerToJITTargetAddress(&commit_null), JITSymbolFlags());

            cantFail(jd.define(absoluteSymbols(symbols)));
        }

        static void WriteOptimizedToFile(llvm::Module const &M, string fileName) {
            std::error_code Error;
            llvm::raw_fd_ostream Out(fileName, Error, llvm::sys::fs::F_None);
            Out << M;
        }

        static Expected<ThreadSafeModule>
        optimizeModule(ThreadSafeModule TSM, const MaterializationResponsibility &R) {
            TSM.withModuleDo([](Module &M) {

                llvm::PassManagerBuilder Builder;
                Builder.OptLevel = 3;
                Builder.Inliner = createFunctionInliningPass(3, 0, false);

                llvm::legacy::PassManager MPM;
                Builder.populateModulePassManager(MPM);

                //WriteOptimizedToFile(M, "unoptmized"); 
                MPM.run(M);
                //WriteOptimizedToFile(M, "optmized"); 
            });

            return std::move(TSM);
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