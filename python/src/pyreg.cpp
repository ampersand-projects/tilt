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

PyReg::PyReg(idx_t size, DataType schema) :
    max_size(get_buf_size(size)),
    schema(schema),
    schema_padding(LLVMGen::GetPadding(schema))
{
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

template<typename T>
void append_btype_data_to_sstream(ostringstream &os,
                                  char* fetch)
{
    T payload = *reinterpret_cast<T*>(fetch);
    os << to_string(payload);
}

void append_data_to_sstream(ostringstream &os, const DataType& dt,
                            const PaddingInfo& padding_info, char* fetch)
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
                offset = padding_info.offsets[i];
                if (dt.dtypes[i].btype == BaseType::STRUCT) {
                    append_data_to_sstream(os,
                                           dt.dtypes[i],
                                           padding_info.nested_padding.at(i),
                                           fetch + offset);
                } else {
                    append_data_to_sstream(os,
                                           dt.dtypes[i],
                                           padding_info,
                                           fetch + offset);
                }
                os << ",";
            }

            os.seekp(-1, os.cur);
            os << "]";
            break;
        }
        default: throw runtime_error("Invalid type.");
    }
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
        append_data_to_sstream(os, schema, schema_padding,
                               fetch(reg.get(), tl_i.t + tl_i.d, i, payload_bytes));
        os << "]";
    }
    os << "]" << endl;

    return os.str();
}

template<typename T>
void write_btype_data_to_ptr(py::object payload, char* raw_data_ptr)
{
    T* data_ptr = reinterpret_cast<T*>(raw_data_ptr);
    *data_ptr = payload.cast<T>();
}

void write_data_to_ptr(py::object payload, const DataType& dt,
                       const PaddingInfo& padding_info, char* raw_data_ptr)
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
                offset = padding_info.offsets[i];
                if (dt.dtypes[i].btype == BaseType::STRUCT) {
                    write_data_to_ptr(payload_list[i],
                                    dt.dtypes[i],
                                    padding_info.nested_padding.at(i),
                                    raw_data_ptr + offset);
                } else {
                    write_data_to_ptr(payload_list[i],
                                    dt.dtypes[i],
                                    padding_info,
                                    raw_data_ptr + offset);
                }
            }
            break;
        }
        default: throw runtime_error("Invalid type. " + dt.str());
    }
}

void PyReg::write_data(py::object payload, ts_t t, idx_t i)
{
    write_data_to_ptr(payload,
                      schema,
                      schema_padding,
                      fetch(reg.get(), t, i,
                            schema_padding.total_bytes));
}

py::object get_payload_from_ptr(char* data_ptr, const DataType& dt,
                                const PaddingInfo& padding_info)
{
    switch (dt.btype) {
        case BaseType::INT8:
            return py::int_(*reinterpret_cast<int8_t*>(data_ptr));
        case BaseType::INT16:
            return py::int_(*reinterpret_cast<int16_t*>(data_ptr));
        case BaseType::INT32:
            return py::int_(*reinterpret_cast<int32_t*>(data_ptr));
        case BaseType::INT64:
            return py::int_(*reinterpret_cast<int64_t*>(data_ptr));
        case BaseType::UINT8:
            return py::int_(*reinterpret_cast<uint8_t*>(data_ptr));
        case BaseType::UINT16:
            return py::int_(*reinterpret_cast<uint16_t*>(data_ptr));
        case BaseType::UINT32:
            return py::int_(*reinterpret_cast<uint32_t*>(data_ptr));
        case BaseType::UINT64:
            return py::int_(*reinterpret_cast<uint64_t*>(data_ptr));
        case BaseType::FLOAT32:
            return py::float_(*reinterpret_cast<float*>(data_ptr));
        case BaseType::FLOAT64:
            return py::float_(*reinterpret_cast<double*>(data_ptr));
        case BaseType::STRUCT: {
            py::list ret;
            uint64_t offset;
            for (int i = 0; i < dt.dtypes.size(); i++) {
                offset = padding_info.offsets[i];
                if (dt.dtypes[i].btype == BaseType::STRUCT) {
                    ret.append(get_payload_from_ptr(data_ptr + offset,
                                                    dt.dtypes[i],
                                                    padding_info.nested_padding.at(i)));
                } else {
                    ret.append(get_payload_from_ptr(data_ptr + offset,
                                                    dt.dtypes[i],
                                                    padding_info));
                }
            }
            return ret;
        }
        default: throw runtime_error("Invalid type. " + dt.str());
    }
}

py::object PyReg::get_payload(idx_t i)
{
    char* data_ptr = reg->data + (i * schema_padding.total_bytes);
    return get_payload_from_ptr(data_ptr, schema, schema_padding);
}
