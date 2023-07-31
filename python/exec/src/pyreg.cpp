#include "pyreg.h"

#include <pybind11/pybind11.h>

#include <memory>
#include <string>
#include <sstream>
#include <stdexcept>

#include "tilt/base/ctype.h"
#include "tilt/base/type.h"
#include "tilt/pass/codegen/vinstr.h"
#include "tilt/pass/codegen/llvmgen.h"

using namespace std;
using namespace tilt;

namespace py = pybind11;

PyReg::PyReg(idx_t size,
             shared_ptr<DataType> schema)
{
    this->max_size = get_buf_size(size);
    this->schema = schema;
    this->schema_padding = LLVMGen::getStructPadding(*schema);

    this->reg = make_unique<region_t>();
    ival_t* tl = new ival_t[max_size];
    char* data = new char[max_size * schema_padding.total_bytes];
    init_region(this->reg.get(), 0, this->max_size, tl, data);
}

PyReg::~PyReg(void)
{
    delete [] this->reg->tl;
    delete [] this->reg->data;
}

region_t* PyReg::get_reg(void)
{
    return this->reg.get();
}

string PyReg::str(void)
{
    ostringstream os;

    os << "TiLT region object" << endl;

    os << "Metadata:" << endl;
    os << "st: " << reg->st << endl;
    os << "et: " << reg->et << endl;
    os << "head: " << reg->head << endl;
    os << "count: " << reg->count << endl;

    os << "Timestamps and Data:" << endl;
    os << "[";
    uint32_t payload_bytes = schema_padding.total_bytes;
    for (int i = 0; i < reg->count; i++) {
        ival_t tl_i = reg->tl[i];
        os << "[";
        os << "[" << tl_i.t << "," << tl_i.d << "]" << ",";
        append_data_to_sstream(os, *schema, fetch(reg.get(), tl_i.t + tl_i.d, i, payload_bytes));
        os << "]";
    }
    os << "]" << endl;

    return os.str();
}

void PyReg::append_data_to_sstream(ostringstream &os, DataType dt, char* fetch)
{
    switch (dt.btype) {
        case BaseType::INT8:
            append_btype_data_to_sstream<int8_t>(os, fetch);
            break;
        case BaseType::INT16:
            append_btype_data_to_sstream<int16_t>(os, fetch);
            break;
        case BaseType::INT32:
            append_btype_data_to_sstream<int32_t>(os, fetch);
            break;
        case BaseType::INT64:
            append_btype_data_to_sstream<int64_t>(os, fetch);
            break;
        case BaseType::UINT8:
            append_btype_data_to_sstream<uint8_t>(os, fetch);
            break;
        case BaseType::UINT16:
            append_btype_data_to_sstream<uint16_t>(os, fetch);
            break;
        case BaseType::UINT32:
            append_btype_data_to_sstream<uint32_t>(os, fetch);
            break;
        case BaseType::UINT64:
            append_btype_data_to_sstream<uint64_t>(os, fetch);
            break;
        case BaseType::FLOAT32:
            append_btype_data_to_sstream<float>(os, fetch);
            break;
        case BaseType::FLOAT64:
            append_btype_data_to_sstream<double>(os, fetch);
            break;
        case BaseType::STRUCT: {
            os << "[";

            uint64_t offset;
            for (int i = 0; i < dt.dtypes.size(); ++i) {
                offset = schema_padding.offsets[i];
                append_data_to_sstream(os, dt.dtypes[i], fetch + offset);
                os << ",";
            }

            os.seekp(-1, os.cur);
            os << "]";
            break;
        }
        default: throw runtime_error("Invalid type.");
    }
}

template<typename T>
void PyReg::append_btype_data_to_sstream(ostringstream &os,
                                         char* fetch)
{
    T payload = *reinterpret_cast<T*>(fetch);
    os << to_string(payload);
}

void PyReg::write_data(py::object payload, ts_t t, idx_t i)
{
    write_data_to_ptr(payload,
                      *schema,
                      fetch(reg.get(), t, i,
                            schema_padding.total_bytes));
}

void PyReg::write_data_to_ptr(py::object payload, DataType dt, char* raw_data_ptr)
{
    switch (dt.btype) {
        case BaseType::INT8:
            write_btype_data_to_ptr<int8_t>(payload, raw_data_ptr);
            break;
        case BaseType::INT16:
            write_btype_data_to_ptr<int16_t>(payload, raw_data_ptr);
            break;
        case BaseType::INT32:
            write_btype_data_to_ptr<int32_t>(payload, raw_data_ptr);
            break;
        case BaseType::INT64:
            write_btype_data_to_ptr<int64_t>(payload, raw_data_ptr);
            break;
        case BaseType::UINT8:
            write_btype_data_to_ptr<uint8_t>(payload, raw_data_ptr);
            break;
        case BaseType::UINT16:
            write_btype_data_to_ptr<uint16_t>(payload, raw_data_ptr);
            break;
        case BaseType::UINT32:
            write_btype_data_to_ptr<uint32_t>(payload, raw_data_ptr);
            break;
        case BaseType::UINT64:
            write_btype_data_to_ptr<uint64_t>(payload, raw_data_ptr);
            break;
        case BaseType::FLOAT32:
            write_btype_data_to_ptr<float>(payload, raw_data_ptr);
            break;
        case BaseType::FLOAT64:
            write_btype_data_to_ptr<double>(payload, raw_data_ptr);
            break;
        case BaseType::STRUCT: {
            py::list payload_list = payload.cast<py::list>();

            uint64_t offset;
            for (int i = 0; i < dt.dtypes.size(); i++) {
                offset = schema_padding.offsets[i];
                write_data_to_ptr(payload_list[i],
                                  dt.dtypes[i],
                                  raw_data_ptr + offset);
            }
            break;
        }
        default: throw runtime_error("Invalid type. " + dt.str());
    }
}

template<typename T>
void PyReg::write_btype_data_to_ptr(py::object payload, char* raw_data_ptr)
{
    T* data_ptr = reinterpret_cast<T*>(raw_data_ptr);
    *data_ptr = payload.cast<T>();
}
