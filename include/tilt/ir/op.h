#ifndef INCLUDE_TILT_IR_OP_H_
#define INCLUDE_TILT_IR_OP_H_

#include <map>
#include <functional>
#include <utility>
#include <memory>

#include "tilt/ir/expr.h"
#include "tilt/ir/lstream.h"

using namespace std;

namespace tilt {

struct OpNode : public LStream {
    Iter iter;
    Params inputs;
    SymTable syms;
    Expr pred;
    Sym output;

    OpNode(Iter iter, Params inputs, SymTable syms, Expr pred, Sym output) :
        LStream(Type(output->type.dtype, iter)), iter(iter),
        inputs(move(inputs)), syms(move(syms)), pred(pred), output(output)
    {}

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
        ASSERT(op->output->type.is_valtype());
    }

    void Accept(Visitor&) const final;
};

}  // namespace tilt


#endif  // INCLUDE_TILT_IR_OP_H_
