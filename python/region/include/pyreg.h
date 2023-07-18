#ifndef PYTHON_REGION_INCLUDE_PYREG_H_
#define PYTHON_REGION_INCLUDE_PYREG_H_

#include <pybind11/pybind11.h>

#include <memory>
#include <string>
#include <sstream>

#include "tilt/base/ctype.h"
#include "tilt/base/type.h"
#include "tilt/pass/codegen/llvmtype.h"

namespace py = pybind11;

class PyReg {
public:
    PyReg(idx_t size,
          std::shared_ptr<tilt::DataType> schema);
    ~PyReg();

    region_t* get_reg(void);
    std::string str(void);
    void write_data(py::object payload, ts_t t, idx_t i);

private:
    /* PyReg::str helpers */
    void append_data_to_sstream(std::ostringstream &os, tilt::DataType dt, char* fetch);
    template<typename T>
    void append_btype_data_to_sstream(std::ostringstream &os, char *fetch);

    /* PyReg::write_data helpers */
    void write_data_to_ptr(py::object payload,
                           tilt::DataType dt,
                           char* raw_data_ptr);
    template<typename T>
    void write_btype_data_to_ptr(py::object payload, char* raw_data_ptr);

    /* PyReg members */
    std::unique_ptr<region_t> reg;
    uint32_t max_size;
    std::shared_ptr<tilt::DataType> schema;
    tilt::StructPaddingInfo schema_padding;
};

#endif  // PYTHON_REGION_INCLUDE_PYREG_H_
