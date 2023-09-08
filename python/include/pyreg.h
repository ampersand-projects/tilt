#ifndef PYTHON_INCLUDE_PYREG_H_
#define PYTHON_INCLUDE_PYREG_H_

#include <pybind11/pybind11.h>

#include <memory>
#include <string>
#include <sstream>

#include "tilt/base/ctype.h"
#include "tilt/base/type.h"
#include "tilt/pass/codegen/llvmgen.h"

namespace py = pybind11;

class PyReg {
public:
    PyReg(idx_t size, tilt::DataType schema, ts_t t);
    ~PyReg();
    PyReg(const PyReg&) = delete;

    region_t* get_reg(void);
    std::string str(void);
    void write_data(py::object payload, ts_t t, idx_t i);
    py::object get_payload(idx_t i);

private:
    std::unique_ptr<region_t> reg;
    uint32_t max_size;
    tilt::DataType schema;
    tilt::PaddingInfo schema_padding;
};

#endif  // PYTHON_INCLUDE_PYREG_H_
