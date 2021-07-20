#ifndef TILT_TYPE
#define TILT_TYPE

#include <utility>
#include <vector>
#include <cstdint>
#include <memory>
#include <string>
#include <cassert>

using namespace std;

namespace tilt {

    enum class BaseType {
        BOOL,
        CHAR,
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
    };

    struct DataType {
        const BaseType btype;
        const vector<DataType> dtypes;
        const size_t size;

        DataType(BaseType btype, vector<DataType> dtypes, size_t size = 0) :
            btype(btype), dtypes(dtypes), size(size)
        {
            switch (btype) {
                case BaseType::STRUCT: assert(dtypes.size() > 0); break;
                case BaseType::PTR: assert(dtypes.size() == 1); break;
                default: assert(dtypes.size() == 0); break;
            }
        }

        DataType(BaseType btype, size_t size = 0) :
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
                || (this->btype == BaseType::FLOAT64);
        }
    };

    struct Iter {
        long offset;
        long period;
        string name;

        Iter(long offset, long period) :
            Iter(offset, period, "(" + to_string(offset) + "," + to_string(period) + ")")
        {}

        Iter(string name) : Iter(0, -1, name) {}

        Iter() : offset(0), period(0), name("") {}

        bool operator==(const Iter& o) const
        {
            return (this->offset == o.offset)
                && (this->period == o.period)
                && (this->name == o.name);
        }

    private:
        Iter(long offset, long period, string name) :
            offset(offset), period(period), name("~" + name)
        {}
    };

    struct Type {
        const DataType dtype;
        const Iter iter;

        Type(DataType dtype, Iter iter) :
            dtype(move(dtype)), iter(iter)
        {}

        Type(DataType dtype) : Type(move(dtype), Iter()) {}

        bool is_valtype() const { return iter.period == 0; }

        bool operator==(const Type& o) const
        {
            return (this->dtype == o.dtype)
                && (this->iter == o.iter);
        }
    };

    enum class MathOp {
        ADD, SUB, MUL, DIV, MAX, MIN,
        LT, LTE, GT, GTE, EQ,
        NOT, AND, OR,
    };

    namespace types {

        static const DataType BOOL = { BaseType::BOOL };
        static const DataType CHAR = { BaseType::CHAR };
        static const DataType CHAR_PTR = DataType(BaseType::PTR, {types::CHAR});
        static const DataType INT8 = { BaseType::INT8 };
        static const DataType INT16 = { BaseType::INT16 };
        static const DataType INT32 = { BaseType::INT32 };
        static const DataType INT64 = { BaseType::INT64 };
        static const DataType UINT8 = { BaseType::UINT8 };
        static const DataType UINT16 = { BaseType::UINT16 };
        static const DataType UINT32 = { BaseType::UINT32 };
        static const DataType UINT64 = { BaseType::UINT64 };
        static const DataType FLOAT32 = { BaseType::FLOAT32 };
        static const DataType FLOAT64 = { BaseType::FLOAT64 };

        // Loop IR types
        static const DataType TIME = { BaseType::TIME };
        static const DataType INDEX = { BaseType::INDEX };
        static const DataType INDEX_PTR = DataType(BaseType::PTR, {types::INDEX});

        template<typename H> struct Converter { static const BaseType btype = BaseType::UNKNOWN; };
        template<> struct Converter<bool> { static const BaseType btype = BaseType::BOOL; };
        template<> struct Converter<char> { static const BaseType btype = BaseType::CHAR; };
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
        static void convert(vector<BaseType>& btypes) {}

        template<size_t n, typename H, typename... Ts>
        static void convert(vector<BaseType>& btypes)
        {
            btypes[n - sizeof...(Ts) - 1] = Converter<H>::btype;
            convert<n, Ts...>(btypes);
        }

        template<typename... Ts>
        DataType STRUCT()
        {
            vector<BaseType> btypes(sizeof...(Ts));
            convert<sizeof...(Ts), Ts...>(btypes);

            vector<DataType> dtypes;
            for (const auto& btype: btypes) {
                dtypes.push_back(DataType(btype));
            }
            return DataType(BaseType::STRUCT, dtypes);
        }
    } // namespace types

} // namespace tilt

extern "C" {

    struct index_t {
        long t;
        unsigned int i;
    };

    struct region_t {
        index_t si;
        index_t ei;
        index_t* tl;
        char* data;
    };

}

#endif // TILT_TYPE
