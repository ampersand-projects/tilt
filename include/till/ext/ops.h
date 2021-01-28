#ifndef TILL_OPS
#define TILL_OPS

#include "till/core/expr.h"
#include "till/core/op.h"
#include "till/ext/lstreams.h"

using namespace std;

namespace till {

    template<typename O, typename I>
    class UnaryElemOp : public ElemOp<O, I> {
    public:
        UnaryElemOp(TLPtr timeline, LambdaPtr<O, I> expr, LSPtr<I> lstream, PtPtr pt) :
            ElemOp<O, I>{ timeline, expr, make_shared<Element<I>>(pt, lstream) }
        {}
    }; // class UnaryElemOp

    template<typename O, typename I>
    class SelectOp : public UnaryElemOp<O, I> {
    public:
        SelectOp(const string name, LambdaPtr<O, I> expr, LSPtr<I> in) :
            UnaryElemOp<O, I>{ make_shared<PeriodLine<0,1>>(name),
                               expr, in, make_shared<CurPt>(name + "_cur") }
        {}
    }; // class SelectOp

    /*
    template<typename T>
    class WhereOp : public UnaryElemOp<T, T> {
    }; // class WhereOp

    template<typename O, typename L, typename R>
    class BinaryElemOp : public ElemOp<O, L, R> {
    }; // class BinaryElemOp

    template<typename O, typename L, typename R>
    class JoinOp : public BinaryElemOp<O, L, R> {
    }; // class JoinOp

    template<typename T>
    class SumOp : public AggOp<T, T> {
    }; // class SumOp
    */

} // namespace till

#endif // TILL_OPS