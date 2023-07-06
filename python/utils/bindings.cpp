#include <pybind11/pybind11.h>

#include "tilt/builder/tilder.h"
#include "tilt/ir/op.h"
#include "tilt/pass/printer.h"
#include "tilt/pass/codegen/loopgen.h"

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

PYBIND11_MODULE(utils, m) {
    m.def("print_IR", &print_IR);
}
