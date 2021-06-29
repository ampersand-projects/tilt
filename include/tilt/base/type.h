#ifndef TILT_TYPE
#define TILT_TYPE

#include <utility>
#include <vector>
#include <cstdint>
#include <memory>
#include <string>

using namespace std;

namespace tilt {

    enum class PrimitiveType {
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
        TIMESTAMP,
        UNKNOWN,

        // Loop IR types
        TIME,
        INDEX,
    };

    struct DataType {
        const vector<PrimitiveType> ptypes;
        const bool is_ptr;

        DataType(vector<PrimitiveType> ptypes, bool is_ptr = false) :
            ptypes(ptypes), is_ptr(is_ptr)
        {}
        DataType(PrimitiveType ptype, bool is_ptr = false) :
            DataType(vector<PrimitiveType>{ ptype }, is_ptr)
        {}

        bool operator==(const DataType& o) const
        {
            return (this->ptypes == o.ptypes)
                && (this->is_ptr == o.is_ptr);
        }
    };

    struct Iter {
        long offset;
        long period;
        string name;

        Iter(long offset, long period, string name) :
            offset(offset), period(period), name("~" + name)
        {}

        Iter(long offset, long period) :
            Iter(offset, period, "(" + to_string(offset) + "," + to_string(period) + ")")
        {}

        bool operator==(const Iter& o) const
        {
            return (this->offset == o.offset)
                && (this->period == o.period)
                && (this->name == o.name);
        }
    };

    struct FreqIter : public Iter {
        FreqIter(long offset, long period) : Iter(offset, period) {}
    };

    struct FreeIter : public Iter {
        FreeIter(string name) : Iter(0, -1, name) {}
    };

    struct Timeline {
        vector<Iter> iters;

        Timeline(vector<Iter> iters) : iters(iters) {}
        Timeline(Iter iter) : Timeline({iter}) {}
        Timeline(std::initializer_list<Iter> iters) : Timeline(vector<Iter>(iters)) {}
        Timeline() {}

        bool operator==(const Timeline& o) const
        {
            return this->iters == o.iters;
        }
    };

    struct Type {
        const DataType dtype;
        const Timeline tl;

        Type(DataType dtype, Timeline tl) :
            dtype(dtype), tl(tl)
        {}

        Type(DataType dtype) : Type(dtype, Timeline()) {}

        Type(DataType dtype, Iter iter) :
            Type(dtype, Timeline(iter))
        {}

        bool isLStream() const { return tl.iters.size() > 0; }

        bool operator==(const Type& o) const
        {
            return (this->dtype == o.dtype)
                && (this->tl == o.tl);
        }
    };

    namespace types {

        static const DataType BOOL = { PrimitiveType::BOOL };
        static const DataType CHAR = { PrimitiveType::CHAR };
        static const DataType INT8 = { PrimitiveType::INT8 };
        static const DataType INT16 = { PrimitiveType::INT16 };
        static const DataType INT32 = { PrimitiveType::INT32 };
        static const DataType INT64 = { PrimitiveType::INT64 };
        static const DataType UINT8 = { PrimitiveType::UINT8 };
        static const DataType UINT16 = { PrimitiveType::UINT16 };
        static const DataType UINT32 = { PrimitiveType::UINT32 };
        static const DataType UINT64 = { PrimitiveType::UINT64 };
        static const DataType FLOAT32 = { PrimitiveType::FLOAT32 };
        static const DataType FLOAT64 = { PrimitiveType::FLOAT64 };
        static const DataType TIMESTAMP = { PrimitiveType::TIMESTAMP };

        // Loop IR types
        static const DataType TIME = vector<PrimitiveType>{ PrimitiveType::TIME };
        static const DataType INDEX = vector<PrimitiveType>{ PrimitiveType::INDEX };
        static const DataType INDEX_PTR = DataType(vector<PrimitiveType>{ PrimitiveType::INDEX }, true);

        template<typename H> struct Converter { static const PrimitiveType btype = PrimitiveType::UNKNOWN; };
        template<> struct Converter<bool> { static const PrimitiveType btype = PrimitiveType::BOOL; };
        template<> struct Converter<char> { static const PrimitiveType btype = PrimitiveType::CHAR; };
        template<> struct Converter<int8_t> { static const PrimitiveType btype = PrimitiveType::INT8; };
        template<> struct Converter<int16_t> { static const PrimitiveType btype = PrimitiveType::INT16; };
        template<> struct Converter<int32_t> { static const PrimitiveType btype = PrimitiveType::INT32; };
        template<> struct Converter<int64_t> { static const PrimitiveType btype = PrimitiveType::INT64; };
        template<> struct Converter<uint8_t> { static const PrimitiveType btype = PrimitiveType::UINT8; };
        template<> struct Converter<uint16_t> { static const PrimitiveType btype = PrimitiveType::UINT16; };
        template<> struct Converter<uint32_t> { static const PrimitiveType btype = PrimitiveType::UINT32; };
        template<> struct Converter<uint64_t> { static const PrimitiveType btype = PrimitiveType::UINT64; };
        template<> struct Converter<float> { static const PrimitiveType btype = PrimitiveType::FLOAT32; };
        template<> struct Converter<double> { static const PrimitiveType btype = PrimitiveType::FLOAT64; };

        template<size_t n, typename H, typename... Ts>
        static constexpr void convert(vector<PrimitiveType>& btypes)
        {
            btypes[n - sizeof...(Ts) - 1] = Converter<H>::btype;
            convert<n, Ts...>(btypes);
        }

        template<size_t n>
        static constexpr void convert(vector<PrimitiveType>& btypes) {}

        template<typename... Ts>
        const DataType BuildType()
        {
            vector<PrimitiveType> btypes(sizeof...(Ts));
            convert<sizeof...(Ts), Ts...>(btypes);
            return DataType(btypes);
        }
    } // namespace types

} // namespace tilt

#endif // TILT_TYPE