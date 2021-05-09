#ifndef TILT_TYPE
#define TILT_TYPE

#include <utility>
#include <vector>
#include <cstdint>
#include <memory>
#include <string>

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
        TIMESTAMP,
        UNKNOWN,

        // Loop IR types
        TIME,
        INDEX,
    };

    struct DataType {
        const vector<BaseType> btypes;

        DataType(vector<BaseType> btypes) : btypes(btypes) {}
        DataType(BaseType btype) :
            DataType(move(vector<BaseType>{ btype }))
        {}

        bool operator==(const DataType& o) const
        {
            return (this->btypes == o.btypes);
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

        Timeline(vector<Iter> iters) : iters(move(iters)) {}
        Timeline(Iter iter) : Timeline({iter}) {}
        Timeline(std::initializer_list<Iter> iters) : Timeline(move(vector<Iter>(iters))) {}
        Timeline() {}
    };

    struct Type {
        const DataType dtype;
        const Timeline tl;

        Type(DataType dtype, Timeline tl) :
            dtype(move(dtype)), tl(move(tl))
        {}

        Type(DataType dtype) : Type(move(dtype), Timeline()) {}

        Type(DataType dtype, Iter iter) :
            Type(move(dtype), Timeline(iter))
        {}
    };

    namespace types {

        static const DataType BOOL = { BaseType::BOOL };
        static const DataType CHAR = { BaseType::CHAR };
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
        static const DataType TIMESTAMP = { BaseType::TIMESTAMP };

        // Loop IR types
        static const DataType TIME = vector<BaseType>{ BaseType::TIME };
        static const DataType INDEX = vector<BaseType>{ BaseType::INDEX };

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

        template<size_t n, typename H, typename... Ts>
        static constexpr void convert(vector<BaseType>& btypes)
        {
            btypes[n - sizeof...(Ts) - 1] = Converter<H>::btype;
            convert<n, Ts...>(btypes);
        }

        template<size_t n>
        static constexpr void convert(vector<BaseType>& btypes) {}

        template<typename... Ts>
        constexpr DataType BuildType()
        {
            vector<BaseType> btypes(sizeof...(Ts));
            convert<sizeof...(Ts), Ts...>(btypes);
            return DataType(btypes);
        }
    } // namespace types

} // namespace tilt

#endif // TILT_TYPE