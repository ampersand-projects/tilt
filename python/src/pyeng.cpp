#include <string>
#include <vector>
#include <utility>

#include "pyeng.h"
#include "pyreg.h"

#include "tilt/base/ctype.h"
#include "tilt/builder/tilder.h"
#include "tilt/engine/engine.h"
#include "tilt/ir/op.h"
#include "tilt/pass/codegen/loopgen.h"
#include "tilt/pass/codegen/llvmgen.h"

using namespace std;
using namespace tilt;
using namespace tilt::tilder;

PyEng::PyEng(void)
{
    this->jit = ExecEngine::Get();
}

intptr_t PyEng::compile(Op query_op, string query_name)
{
    auto query_op_sym = _sym(query_name, query_op);

    auto loop = LoopGen::Build(query_op_sym, query_op.get());

    auto& llctx = jit->GetCtx();
    auto llmod = LLVMGen::Build(loop, llctx);
    jit->AddModule(move(llmod));
    auto addr = jit->Lookup(loop->get_name());

    return addr;
}

template<typename... Args>
void PyEng::execute_va(intptr_t addr, ts_t t_start, ts_t t_end,
                       region_t* out_reg, Args... in_regs)
{
    auto query = (region_t* (*)(ts_t, ts_t, region_t*, ...)) addr;
    query(t_start, t_end, out_reg, (in_regs)...);
}

void PyEng::execute(intptr_t addr, ts_t t_start, ts_t t_end, PyReg* out_reg, PyReg* in_reg)
{
    execute_va(addr, t_start, t_end, out_reg->get_reg(), in_reg->get_reg());
}

void PyEng::execute(intptr_t addr, ts_t t_start, ts_t t_end, PyReg* out_reg,
                    PyReg* in_reg_0, PyReg* in_reg_1)
{
    execute_va(addr, t_start, t_end, out_reg->get_reg(),
               in_reg_0->get_reg(), in_reg_1->get_reg());
}
