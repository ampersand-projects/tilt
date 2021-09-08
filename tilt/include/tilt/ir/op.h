#ifndef TILT_INCLUDE_TILT_IR_OP_H_
#define TILT_INCLUDE_TILT_IR_OP_H_

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
    Aux aux;

    OpNode(Iter iter, Params inputs, SymTable syms, Expr pred, Sym output, Aux aux = {}) :
        LStream(Type(output->type.dtype, iter)), iter(iter),
        inputs(move(inputs)), syms(move(syms)), pred(pred), output(output), aux(move(aux))
    {}

    void Accept(Visitor&) const final;
};
typedef shared_ptr<OpNode> Op;

// Accumulate function type (state, st, et, data) -> state
typedef function<Expr(Expr, Expr, Expr, Expr)> AccTy;

struct Reduce : public ValNode {
    Sym lstream;
    Val state;
    AccTy acc;

    Reduce(Sym lstream, Val state, AccTy acc) :
        ValNode(state->type.dtype), lstream(lstream), state(state), acc(acc)
    {
        auto st = make_shared<Symbol>("st", Type(types::TIME));
        auto et = make_shared<Symbol>("et", Type(types::TIME));
        auto data = make_shared<Symbol>("data", Type(lstream->type.dtype));
        ASSERT(acc(state, st, et, data)->type == Type(state->type.dtype));
    }

    void Accept(Visitor&) const final;
};

}  // namespace tilt


#endif  // TILT_INCLUDE_TILT_IR_OP_H_
