#ifndef INCLUDE_TILT_BASE_TYPE_H_
#define INCLUDE_TILT_BASE_TYPE_H_

#include <utility>
#include <vector>
#include <memory>
#include <string>

#include "tilt/base/ctype.h"
#include "tilt/base/log.h"

using namespace std;

namespace tilt {

enum class BaseType {
    BOOL,
    INT8,
    INT16,
    INT32,
    INT64,
    UINT8,
    UINT16,
    UINT32,
    UINT64,
    FLOAT32,
    FLOAT64,
    STRUCT,
    PTR,
    UNKNOWN,

    // Loop IR types
    TIME,
    INDEX,
    IVAL,
};

struct DataType {
    const BaseType btype;
    const vector<DataType> dtypes;
    const size_t size;

    DataType(BaseType btype, vector<DataType> dtypes, size_t size = 0) :
        btype(btype), dtypes(dtypes), size(size)
    {
        switch (btype) {
            case BaseType::STRUCT: ASSERT(dtypes.size() > 0); break;
            case BaseType::PTR: ASSERT(dtypes.size() == 1); break;
            default: ASSERT(dtypes.size() == 0); break;
        }
    }

    explicit DataType(BaseType btype, size_t size = 0) :
        DataType(btype, {}, size)
    {}

    bool operator==(const DataType& o) const
    {
        return (this->btype == o.btype)
            && (this->dtypes == o.dtypes)
            && (this->size == o.size);
    }

    bool is_struct() const { return btype == BaseType::STRUCT; }
    bool is_ptr() const { return btype == BaseType::PTR; }
    bool is_arr() const { return size > 0; }

    bool is_float() const
    {
        return (this->btype == BaseType::FLOAT32)
            || (this->btype == BaseType::FLOAT64);
    }

    bool is_int() const
    {
        return (this->btype == BaseType::INT8)
            || (this->btype == BaseType::INT16)
            || (this->btype == BaseType::INT32)
            || (this->btype == BaseType::INT64)
            || (this->btype == BaseType::UINT8)
            || (this->btype == BaseType::UINT16)
            || (this->btype == BaseType::UINT32)
            || (this->btype == BaseType::UINT64);
    }

    bool is_signed() const
    {
        return (this->btype == BaseType::INT8)
            || (this->btype == BaseType::INT16)
            || (this->btype == BaseType::INT32)
            || (this->btype == BaseType::INT64)
            || (this->btype == BaseType::FLOAT32)
            || (this->btype == BaseType::FLOAT64)
            || (this->btype == BaseType::TIME);
    }

    DataType ptr() const { return DataType(BaseType::PTR, {*this}); }

    DataType deref() const
    {
        ASSERT(this->is_ptr());
        return this->dtypes[0];
    }

    string str() const
    {
        switch (btype) {
            case BaseType::BOOL: return "b";
            case BaseType::INT8: return "i8";
            case BaseType::UINT8: return "u8";
            case BaseType::INT16: return "i16";
            case BaseType::UINT16: return "u16";
            case BaseType::INT32: return "i32";
            case BaseType::UINT32: return "u32";
            case BaseType::INT64: return "i64";
            case BaseType::UINT64: return "u64";
            case BaseType::FLOAT32: return "f32";
            case BaseType::FLOAT64: return "f64";
            case BaseType::TIME: return "t";
            case BaseType::INDEX: return "x";
            case BaseType::PTR: return "*" + dtypes[0].str();
            case BaseType::STRUCT: {
                string res = "";
                for (const auto& dtype : dtypes) {
                    res += dtype.str() + ", ";
                }
                res.resize(res.size() - 2);
                return "{" + res + "}";
            }
            case BaseType::IVAL:
            case BaseType::UNKNOWN:
            default: throw std::runtime_error("Invalid type");
        }
    }
};

struct Iter {
    int64_t offset;
    int64_t period;

    Iter(int64_t offset, int64_t period) :
        offset(offset), period(period)
    {}

    Iter() : offset(0), period(0) {}

    bool operator==(const Iter& o) const
    {
        return (this->offset == o.offset)
            && (this->period == o.period);
    }

    string str() const { return "(" + to_string(offset) + ", " + to_string(period) + ")"; }
};

struct Type {
    const DataType dtype;
    const Iter iter;

    Type(DataType dtype, Iter iter) :
        dtype(std::move(dtype)), iter(iter)
    {}

    explicit Type(DataType dtype) : Type(std::move(dtype), Iter()) {}

    bool is_val() const { return iter.period == 0; }
    bool is_beat() const { return iter.period > 0 && dtype.btype == BaseType::TIME; }
    bool is_out() const { return iter.period == -2; }

    bool operator==(const Type& o) const
    {
        return (this->dtype == o.dtype)
            && (this->iter == o.iter);
    }

    string str() const { return iter.str() + " " + dtype.str(); }
};

enum class MathOp {
    ADD, SUB, MUL, DIV, MAX, MIN,
    MOD, SQRT, POW,
    ABS, NEG, CEIL, FLOOR,
    LT, LTE, GT, GTE, EQ,
    NOT, AND, OR,
};

}  // namespace tilt

namespace tilt::types {

static const DataType BOOL(BaseType::BOOL);
static const DataType INT8(BaseType::INT8);
static const DataType INT16(BaseType::INT16);
static const DataType INT32(BaseType::INT32);
static const DataType INT64(BaseType::INT64);
static const DataType UINT8(BaseType::UINT8);
static const DataType UINT16(BaseType::UINT16);
static const DataType UINT32(BaseType::UINT32);
static const DataType UINT64(BaseType::UINT64);
static const DataType FLOAT32(BaseType::FLOAT32);
static const DataType FLOAT64(BaseType::FLOAT64);
static const DataType CHAR_PTR = DataType(BaseType::PTR, {types::INT8});
static const DataType TIME(BaseType::TIME);
static const DataType INDEX(BaseType::INDEX);
static const DataType IVAL(BaseType::IVAL);

template<typename H> struct Converter { static const BaseType btype = BaseType::UNKNOWN; };
template<> struct Converter<bool> { static const BaseType btype = BaseType::BOOL; };
template<> struct Converter<char> { static const BaseType btype = BaseType::INT8; };
template<> struct Converter<int8_t> { static const BaseType btype = BaseType::INT8; };
template<> struct Converter<int16_t> { static const BaseType btype = BaseType::INT16; };
template<> struct Converter<int32_t> { static const BaseType btype = BaseType::INT32; };
template<> struct Converter<int64_t> { static const BaseType btype = BaseType::INT64; };
template<> struct Converter<uint8_t> { static const BaseType btype = BaseType::UINT8; };
template<> struct Converter<uint16_t> { static const BaseType btype = BaseType::UINT16; };
template<> struct Converter<uint32_t> { static const BaseType btype = BaseType::UINT32; };
template<> struct Converter<uint64_t> { static const BaseType btype = BaseType::UINT64; };
template<> struct Converter<float> { static const BaseType btype = BaseType::FLOAT32; };
template<> struct Converter<double> { static const BaseType btype = BaseType::FLOAT64; };

template<size_t n>
static void convert(BaseType* btypes) {}

template<size_t n, typename H, typename... Ts>
static void convert(BaseType* btypes)
{
    btypes[n - sizeof...(Ts) - 1] = Converter<H>::btype;
    convert<n, Ts...>(btypes);
}

template<typename... Ts>
DataType STRUCT()
{
    vector<BaseType> btypes(sizeof...(Ts));
    convert<sizeof...(Ts), Ts...>(btypes.data());

    vector<DataType> dtypes;
    for (const auto& btype : btypes) {
        dtypes.push_back(DataType(btype));
    }
    return DataType(BaseType::STRUCT, dtypes);
}

}  // namespace tilt::types

#endif  // INCLUDE_TILT_BASE_TYPE_H_
