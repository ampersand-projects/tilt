#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <memory>
#include <stdexcept>
#include <vector>
#include <iostream>

#include "pyreg.h"
#include "pyeng.h"

#include "tilt/base/ctype.h"
#include "tilt/pass/codegen/vinstr.h"

using namespace std;
using namespace tilt;

namespace py = pybind11;

PYBIND11_MODULE(exec, m) {
    /* region_t wrapper bindings */
    py::class_<PyReg> reg(m, "reg");
    reg.def(py::init<idx_t, DataType>());

    /* implementations for built-in Python methods */
    reg.def("__repr__", &PyReg::str);
    reg.def("__str__", &PyReg::str);

    /* bindings to expose region_t C++ API */
    reg.def("get_start_idx",
            [] (PyReg &pyreg) {
                return get_start_idx(pyreg.get_reg());
            });
    reg.def("get_end_idx",
            [] (PyReg &pyreg) {
                return get_end_idx(pyreg.get_reg());
            });
    reg.def("get_start_time",
            [] (PyReg &pyreg) {
                return get_start_time(pyreg.get_reg());
            });
    reg.def("get_end_time",
            [] (PyReg &pyreg) {
                return get_end_time(pyreg.get_reg());
            });

    reg.def("get_ckpt",
            [] (PyReg &pyreg, ts_t t, idx_t i) {
                return get_ckpt(pyreg.get_reg(), t, i);
            });
    reg.def("advance",
            [] (PyReg &pyreg, idx_t i, ts_t t) {
                return advance(pyreg.get_reg(), i, t);
            });

    reg.def("commit_data",
            [] (PyReg &pyreg, ts_t t) {
                commit_data(pyreg.get_reg(), t);
            });
    reg.def("commit_null",
            [] (PyReg &pyreg, ts_t t) {
                commit_null(pyreg.get_reg(), t);
            });
    reg.def("write_data", &PyReg::write_data);

    /* engine bindings for compilation and execution */
    py::class_<PyEng> engine(m, "engine");
    engine.def(py::init<>());

    engine.def("compile", &PyEng::compile,
          py::arg("query_op"),
          py::arg("query_name") = "query");
    engine.def("execute",
               static_cast<void (PyEng::*)
                                (intptr_t, ts_t, ts_t, PyReg*, PyReg*)>
                          (&PyEng::execute));
    engine.def("execute",
               static_cast<void (PyEng::*)
                                (intptr_t, ts_t, ts_t, PyReg*, PyReg*, PyReg*)>
                          (&PyEng::execute));
}
