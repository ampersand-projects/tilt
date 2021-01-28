#ifndef TILL_OP
#define TILL_OP

#include "till/core/expr.h"
#include "till/core/lstream.h"

#include <tuple>

using namespace std;

namespace till {

    template<typename O>
    class OpBase : public LStream<O> {
    protected:
        OpBase(TLPtr timeline) :
            LStream<O>{ timeline }
        {}
    }; // class OpBase
    template<typename O>
    using OpBasePtr = shared_ptr<OpBase<O>>;

    template<typename O, typename... Is>
    class Op : public OpBase<O> {
    public:
        tuple<Is...> inputs;

    protected:
        Op(TLPtr timeline, Is... inputs) :
            Op<O, Is...>{ timeline, make_tuple(inputs...) }
        {}
        Op(TLPtr timeline, tuple<Is...> inputs) :
            OpBase<O>{ timeline }, inputs(move((inputs)))
        {}
    }; // class Op
    template<typename O, typename... Is>
    using OpPtr = shared_ptr<Op<O, Is...>>;

    template<typename O, typename... Is>
    class ElemOp : public Op<O, ElemPtr<Is>...> {
    public:
        LambdaPtr<O, Is...> expr;

        ElemOp(TLPtr timeline, LambdaPtr<O, Is...> expr, ElemPtr<Is>... inputs) :
            Op<O, ElemPtr<Is>...>{ timeline, inputs... }, expr(move(expr))
        {}
    }; // class ElemOp

    template<typename O, typename I>
    class AggOp : public Op<O, LSPtr<I>> {
    public:
        AggPtr<O, I> agg;

        AggOp(TLPtr timeline, AggPtr<O, I> agg, LSPtr<I> input) :
            Op<O, LSPtr<I>>{ timeline, input }, agg(move(agg))
        {}
    }; // class AggOp

    template<typename O, typename... Is>
    class LStreamOp : public Op<O, LSPtr<Is>...> {
    public:
        OpBasePtr<O> body;

        LStreamOp(TLPtr timeline, OpBasePtr<O> body, LSPtr<Is>... inputs) :
            Op<O, LSPtr<Is>...>{ timeline, inputs... }, body(move(body))
        {}
    }; // class LStreamOp
} // namespace till

#endif // TILL_OP