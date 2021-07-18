#ifndef TILT_OP
#define TILT_OP

#include "tilt/ir/expr.h"
#include "tilt/ir/lstream.h"

#include <map>
#include <functional>

using namespace std;

namespace tilt
{

    struct OpNode : public LStream {
        Iter iter;
        Params inputs;
        SymTable syms;
        Expr pred;
        Sym output;

        OpNode(Iter iter, Params inputs, SymTable syms, Expr pred, Sym output) :
            LStream(Type(output->type.dtype, iter)), iter(iter),
            inputs(move(inputs)), syms(move(syms)), pred(pred), output(output)
        {
            assert(!type.is_valtype());
        }

        void Accept(Visitor&) const final;
    };
    typedef shared_ptr<OpNode> Op;

    typedef function<Expr(Expr, Expr)> AccTy;

    struct AggNode : public ValNode {
        Op op;
        Val init;
        AccTy acc;

        AggNode(Op op, Val init, AccTy acc) :
            ValNode(init->type.dtype), op(op), init(init), acc(acc)
        {
            assert(op->output->type.is_valtype());
        }

        void Accept(Visitor&) const final;
    };

} // namespace tilt


#endif // TILT_OP