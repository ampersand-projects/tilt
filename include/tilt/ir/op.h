#ifndef TILT_OP
#define TILT_OP

#include "tilt/ir/expr.h"
#include "tilt/ir/lstream.h"

#include <map>

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
            LStream(move(Type(output->type.dtype, move(tl)))), iter(iter),
            inputs(move(inputs)), syms(move(syms)), pred(pred), output(output)
        {}

        void Accept(Visitor&) const final;
    };
    typedef shared_ptr<Op> OpPtr;

    enum AggType {
        SUM,
    };

    struct AggExpr : public ValExpr {
        AggType agg;
        OpPtr op;

        AggExpr(DataType dtype, AggType agg, OpPtr op) :
            ValExpr(dtype), agg(agg), op(op)
        {
            assert(op->output->type.tl.iters.size() == 0);
        }

        void Accept(Visitor&) const final;
    };

    struct Sum : public AggExpr {
        Sum(OpPtr op) : AggExpr(op->type.dtype, AggType::SUM, op) {}
    };

} // namespace tilt


#endif // TILT_OP