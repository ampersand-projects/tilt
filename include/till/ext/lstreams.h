#ifndef TILL_LSTREAMS
#define TILL_LSTREAMS

#include "till/core/lstream.h"

#include <string>

using namespace std;

namespace till {

    struct FreeLine : public TimeLine {
        FreeLine(const string name) :
            TimeLine{ name }
        {}
    }; // struct FreeLine

    template<long _offset, ulong _period>
    struct PeriodLine : public TimeLine {
        constexpr long offset() const noexcept { return _offset; }
        constexpr ulong period() const noexcept { return _period; }

        PeriodLine(const string name) :
            TimeLine{ name }
        {}
    }; // struct PeriodLine

    template<long _offset>
    struct ShiftPt : public PtTimeLine {
        constexpr long offset() const noexcept { return _offset; }
        ShiftPt(const string name) :
            PtTimeLine{ name }
        {}
    }; // struct ShiftPt

    struct CurPt : public ShiftPt<0> {
        CurPt(const string name) :
            ShiftPt<0>{ name }
        {}
    }; // struct CurPt

    template<long _begin, long _end>
    struct FixWin : public WinTimeLine {
        constexpr long begin() const noexcept { return _begin; }
        constexpr long end() const noexcept { return _end; }
        FixWin(const string name) :
            WinTimeLine{ name }
        {}
    }; // struct FixWin

} // namespace till

#endif // TILL_LSTREAMS