#ifndef TILT_LSTREAM
#define TILT_LSTREAM

#include "tilt/ir/node.h"

#include <cassert>

using namespace std;

namespace tilt {

    struct LStream : public Expr {
        LStream(Type type) : Expr(move(type)) {}
    };
    typedef shared_ptr<LStream> LSPtr;

    struct Point {
        const long offset;

        Point(long offset) : offset(offset)
        {
            assert(offset <= 0);
        }

        Point() : Point(0) {}

        bool operator<(const Point& o) const
        {
            return offset < o.offset;
        }
    };
    typedef shared_ptr<Point> Pointer;

    struct Window {
        Point start;
        Point end;

        Window(Point start, Point end) :
            start(start), end(end)
        {
            assert(start < end);
        }
    };

    struct SubLStream : public LStream {
        SymPtr lstream;
        const Window win;

        SubLStream(SymPtr lstream, Window win) :
            LStream(lstream->type), lstream(lstream), win(win)
        {}

        void Accept(Visitor&) const final;
    };
    typedef shared_ptr<SubLStream> SubLSPtr;

    struct Element : public ValExpr {
        SymPtr lstream;
        const Point pt;

        Element(SymPtr lstream, Point pt) :
            ValExpr(lstream->type.dtype), lstream(lstream), pt(pt)
        {}

        void Accept(Visitor&) const final;
    };
    typedef shared_ptr<Element> ElemPtr;

} // namespace tilt

#endif // TILT_LSTREAM