#ifndef TILL_LSTREAM
#define TILL_LSTREAM

#include <string>

namespace till {

    struct TimeLine {
    }; // struct TimeLine

    struct FreeLine : public TimeLine {
    }; // struct FreeLine

    template<long _offset, ulong _period>
    struct PeriodLine : public TimeLine {
        constexpr long offset() const noexcept { return _offset; }
        constexpr ulong period() const noexcept { return _period; }
    }; // struct PeriodLine

    template<typename DataType>
    class LStream {
    private:
        std::string name;
        TimeLine timeline;
    }; // class LStream

    template<typename DataType>
    class DataLStream : public LStream<DataType> {
    }; // class DataLStream

    struct Idx {
    }; // struct Idx

    struct PtIdx : public Idx {
    }; // struct PointIdx

    struct WinIdx : public Idx {
    }; // struct WinIdx

    template<long _offset>
    struct ShiftIdx : public PtIdx {
        constexpr long offset() const noexcept { return _offset; }
    }; // struct ShiftIdx

    struct CurIdx : public ShiftIdx<0> {
    }; // struct CurIdx

    template<long _begin, long _end>
    struct FixWinIdx : public WinIdx {
        constexpr long begin() const noexcept { return _begin; }
        constexpr long end() const noexcept { return _end; }
    }; // struct FixWinIdx

    template<typename DataType>
    class SubLStream : public LStream<DataType> {
    private:
        LStream<DataType> lstream;
        WinIdx idx;
    }; // class SubLStream

    template<typename DataType>
    class Element {
    private:
        LStream<DataType> lstream;
        PtIdx idx;
    }; // class Element

} // namespace till

#endif // TILL_LSTREAM