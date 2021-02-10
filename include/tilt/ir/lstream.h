#ifndef TILT_LSTREAM
#define TILT_LSTREAM

#include "tilt/ir/expr.h"

using namespace std;

namespace tilt {

    struct LStream : public Expr {
        LStream(Type type) : Expr(move(type))
        {}
    };
    typedef shared_ptr<LStream> LSPtr;

    struct Window {
        const long shift;
        const unsigned long len;

        Window(long shift, unsigned long len) :
            shift(shift), len(len)
        {}
    };

    struct Point : public Window {
        Point(long offset) : Window(offset, 0) {}
        Point() : Point(0) {}
    };

    struct SubLStream : public LStream {
        ExprPtr lstream;
        const Window win;

        SubLStream(ExprPtr lstream, Window win) :
            LStream(lstream->type), lstream(lstream), win(win)
        {}

        void Accept(Visitor&) const final;
    };

    struct Element : public ValExpr {
        ExprPtr lstream;
        const Point pt;

        Element(ExprPtr lstream, Point pt) :
            ValExpr(lstream->type.dtype), lstream(lstream), pt(pt)
        {}

        void Accept(Visitor&) const final;
    };
    typedef shared_ptr<Element> ElemPtr;

} // namespace tilt

#endif // TILT_LSTREAM