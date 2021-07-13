#ifndef TILT_OP
#define TILT_OP

#include "tilt/ir/expr.h"
#include "tilt/ir/lstream.h"

#include <map>
#include <functional>

using namespace std;

namespace tilt
{

    struct Op : public LStream {
        Iter iter;
        Params inputs;
        SymTable syms;
        PredPtr pred;
        SymPtr output;

        Op(Timeline tl, Iter iter, Params inputs, PredPtr pred, SymTable syms, SymPtr output) :
            LStream(Type(output->type.dtype, move(tl))), iter(iter),
            inputs(move(inputs)), syms(move(syms)), pred(pred), output(output)
        {}

        void Accept(Visitor&) const final;
    };
    typedef shared_ptr<Op> OpPtr;

    typedef function<ExprPtr(ExprPtr, ExprPtr)> AccTy;

    struct AggExpr : public ValExpr {
        OpPtr op;
        ValExprPtr init;
        AccTy acc;

        AggExpr(OpPtr op, ValExprPtr init, AccTy acc) :
            ValExpr(init->type.dtype), op(op), init(init), acc(acc)
        {
            assert(!op->output->type.isLStream());
        }

        void Accept(Visitor&) const final;
    };

} // namespace tilt


#endif // TILT_OP