#ifndef INCLUDE_TILT_IR_LSTREAM_H_
#define INCLUDE_TILT_IR_LSTREAM_H_

#include <utility>
#include <memory>

#include "tilt/ir/node.h"

using namespace std;

namespace tilt {

struct LStream : public ExprNode {
    explicit LStream(Type type) : ExprNode(move(type)) { ASSERT(!this->type.is_val()); }
};

struct Out : public Symbol {
    explicit Out(DataType dtype) : Symbol("", Type(dtype, Iter(0, -1))) {}

    void Accept(Visitor&) const final;
};

struct Beat : public Symbol {
    explicit Beat(Iter iter) : Symbol(iter.str(), Type(types::TIME, iter))
    {
        ASSERT(this->type.is_beat());
    }

    void Accept(Visitor&) const final;
};

struct Point {
    const int64_t offset;

    explicit Point(int64_t offset) : offset(offset) { ASSERT(offset <= 0); }
    Point() : Point(0) {}

    bool operator<(const Point& o) const { return offset < o.offset; }
};
typedef shared_ptr<Point> Pointer;

struct Window {
    Point start;
    Point end;

    Window(Point start, Point end) : start(start), end(end) { ASSERT(start < end); }
    Window(int64_t start, int64_t end) : Window(Point(start), Point(end)) {}
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
    {
        ASSERT(!lstream->type.is_val());
    }

    void Accept(Visitor&) const final;
};

}  // namespace tilt

#endif  // INCLUDE_TILT_IR_LSTREAM_H_
