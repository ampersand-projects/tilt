#include "llvm/IR/Verifier.h"

#include "tilt/engine/engine.h"

using namespace tilt;
using namespace std::placeholders;

ExecEngine* ExecEngine::Get()
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

void ExecEngine::AddModule(unique_ptr<Module> m)
{
    raw_fd_ostream r(fileno(stdout), false);
    verifyModule(*m, &r);

    cantFail(optimizer.add(jd, ThreadSafeModule(move(m), ctx)));
}

LLVMContext& ExecEngine::GetCtx() { return *ctx.getContext(); }

intptr_t ExecEngine::Lookup(StringRef name)
{
    auto fn_sym = cantFail(es.lookup({ &jd }, mangler(name.str())));
    return (intptr_t) fn_sym.getAddress();
}

Expected<ThreadSafeModule> ExecEngine::optimize_module(ThreadSafeModule tsm, const MaterializationResponsibility &r)
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
