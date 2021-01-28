#ifndef TILL_LSTREAM
#define TILL_LSTREAM

#include <string>
#include <memory>

using namespace std;

namespace till {

    struct TimeLine {
        const string name;
        TimeLine(const string name) :
            name(name)
        {}
    }; // struct TimeLine
    using TLPtr = shared_ptr<TimeLine>;

    struct BoundedTimeLine : public TimeLine {
        BoundedTimeLine(const string name) :
            TimeLine{ name }
        {}
    }; // struct BoundedTimeLine
    using BTLPtr = shared_ptr<BoundedTimeLine>;

    struct WinTimeLine : public BoundedTimeLine {
        WinTimeLine(const string name) :
            BoundedTimeLine{ name }
        {}
    }; // struct WinTimeLine
    using WinPtr = shared_ptr<WinTimeLine>;

    struct PtTimeLine : public BoundedTimeLine {
        PtTimeLine(const string name) :
            BoundedTimeLine{ name }
        {}
    }; // struct WinTimeLine
    using PtPtr = shared_ptr<PtTimeLine>;

    template<typename O>
    class LStream {
    public:
        TLPtr timeline;

        LStream(TLPtr timeline) : timeline(timeline)
        {}
    }; // class LStream
    template<typename O>
    using LSPtr = shared_ptr<LStream<O>>;

    template<typename O>
    class SubLStream : public LStream<O> {
    public:
        LSPtr<O> lstream;

        SubLStream(WinPtr win, LSPtr<O> lstream) :
            LStream<O>{ win }, lstream(lstream)
        {}
    }; // class SubLStream

    template<typename O>
    class Element : LStream<O> {
    public:
        LSPtr<O> lstream;

        Element(PtPtr pt, LSPtr<O> lstream) :
            LStream<O>{ pt }, lstream(lstream)
        {}
    }; // class Element
    template<typename O>
    using ElemPtr = shared_ptr<Element<O>>;

} // namespace till

#endif // TILL_LSTREAM