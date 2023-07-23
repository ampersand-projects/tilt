#include <string>
#include <vector>

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

void PyEng::execute(intptr_t addr, ts_t t_start, ts_t t_end,
                    PyReg* out_reg, vector<PyReg*> in_reg_vec)
{
    auto query = (region_t* (*)(ts_t, ts_t, region_t*, ...)) addr;

    switch (in_reg_vec.size()) {
        case 1:
            query(t_start, t_end, out_reg->get_reg(),
                  in_reg_vec[0]->get_reg());
            break;
        case 2:
            query(t_start, t_end, out_reg->get_reg(),
                  in_reg_vec[0]->get_reg(), in_reg_vec[1]->get_reg());
            break;
        default: throw runtime_error("Invalid number of inputs.");
    }
}
