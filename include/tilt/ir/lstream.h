#ifndef TILT_LSTREAM
#define TILT_LSTREAM

#include "tilt/ir/node.h"

#include <cassert>

using namespace std;

namespace tilt {

    struct LStream : public ExprNode {
        LStream(Type type) : ExprNode(move(type)) {}
    };

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
        Sym lstream;
        const Window win;

        SubLStream(Sym lstream, Window win) :
            LStream(lstream->type), lstream(lstream), win(win)
        {}

        void Accept(Visitor&) const final;
    };

    struct Element : public ValNode {
        Sym lstream;
        const Point pt;

        Element(Sym lstream, Point pt) :
            ValNode(lstream->type.dtype), lstream(lstream), pt(pt)
        {}

        void Accept(Visitor&) const final;
    };

} // namespace tilt

#endif // TILT_LSTREAM