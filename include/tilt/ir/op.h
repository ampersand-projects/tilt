#ifndef TILT_OP
#define TILT_OP

#include "tilt/ir/expr.h"
#include "tilt/ir/lstream.h"

using namespace std;

namespace tilt
{

    struct Op : public LStream {
        Params inputs;
        SymTable vars;
        SymPtr output;

        Op(Iter iter, Params inputs, SymTable vars, SymPtr output) :
            LStream(move(Type(output->type.dtype, move(iter)))),
            inputs(move(inputs)), vars(move(vars)), output(output)
        {}

        void Accept(Visitor&) const final;

    protected:
        Op(Type type) : LStream(move(type)) {}
    };
    typedef shared_ptr<Op> OpPtr;

    struct AggExpr : public ValExpr {
        OpPtr op;

        AggExpr(DataType dtype, OpPtr op) :
            ValExpr(dtype), op(op)
        {
            assert(op->inputs.size() == 1);
        }
    };

    struct Sum : public AggExpr {
        Sum(OpPtr op) :
            AggExpr(op->type.dtype, op)
        {}

        void Accept(Visitor&) const final;
    };

} // namespace tilt


#endif // TILT_OP