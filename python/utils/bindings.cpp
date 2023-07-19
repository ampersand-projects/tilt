#include <pybind11/pybind11.h>
#include <string>

#include "tilt/builder/tilder.h"
#include "tilt/engine/engine.h"
#include "tilt/ir/op.h"
#include "tilt/pass/printer.h"
#include "tilt/pass/codegen/loopgen.h"
#include "tilt/pass/codegen/llvmgen.h"

using namespace std;
using namespace tilt;
using namespace tilt::tilder;

namespace py = pybind11;

void print_IR(Op query_op)
{
    cout << "TiLT IR:" << endl;
    cout << IRPrinter::Build(query_op) << endl;

    auto query_op_sym = _sym("query", query_op);
    auto loop = LoopGen::Build(query_op_sym, query_op.get());

    cout << "Loop IR:" << endl;
    cout << IRPrinter::Build(loop);
}

void print_llvmIR(Op query_op, string fname)
{
    auto query_op_sym = _sym("query", query_op);
    auto loop = LoopGen::Build(query_op_sym, query_op.get());

    auto jit = ExecEngine::Get();
    auto& llctx = jit->GetCtx();
    auto llmod = LLVMGen::Build(loop, llctx);

    ofstream f;
    f.open(fname);
    f << IRPrinter::Build(llmod.get());
    f.close();
}

intptr_t compile(Op query_op, string query_name)
{
    auto query_op_sym = _sym(query_name, query_op);

    auto loop = LoopGen::Build(query_op_sym, query_op.get());

    auto jit = ExecEngine::Get();
    auto& llctx = jit->GetCtx();
    auto llmod = LLVMGen::Build(loop, llctx);
    jit->AddModule(move(llmod));
    auto addr = jit->Lookup(loop->get_name());

    return addr;
}

PYBIND11_MODULE(utils, m) {
    m.def("print_IR", &print_IR);
    m.def("print_llvmIR", &print_llvmIR);
    m.def("compile", &compile,
          py::arg("query_op"),
          py::arg("query_name") = "query");

}
