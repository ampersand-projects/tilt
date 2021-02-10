#ifndef TILT_OP
#define TILT_OP

#include "tilt/ir/expr.h"
#include "tilt/ir/lstream.h"

#include <map>

using namespace std;

namespace tilt
{

    struct Op : public LStream {
        Iterator iter;
        Params inputs;
        SymTable syms;
        SymPtr output;

        Op(Timeline tl, Iterator iter, Params inputs, SymTable syms, SymPtr output) :
            LStream(move(Type(output->type.dtype, move(tl)))),
            iter(iter), inputs(move(inputs)), syms(move(syms)), output(output)
        {}

        void Accept(Visitor&) const final;

        Pointer cur, prev;
        map<SymPtr, vector<Pointer>> pointers;
        map<SymPtr, SubLSPtr> subs;
        vector<SymPtr> body;
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