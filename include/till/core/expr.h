#ifndef TILL_EXPR
#define TILL_EXPR

#include "till/core/lstream.h"

#include <memory>

using namespace std;

namespace till {

    template<typename O>
    struct Expr {
    protected:
        Expr() {}
    }; // struct Expr
    template<typename T>
    using ExprPtr = shared_ptr<Expr<T>>;

    template<typename T>
    struct Const : public Expr<T> {
        const T val;
        Const(const T val) : val(move(val)) {}
    }; // struct ConstExpr
    template<typename T>
    using ConstPtr = shared_ptr<Const<T>>;

    template<typename T>
    struct Var : public Expr<T> {
        shared_ptr<T> expr;

        Var() {}
    }; // struct Var
    template<typename T>
    using VarPtr = shared_ptr<Var<T>>;

    template<typename O, typename I>
    struct Agg : public Expr<O> {
        VarPtr<I> var;

        Agg() {}
    }; // struct Agg
    template<typename O, typename I>
    using AggPtr = shared_ptr<Agg<O, I>>;

    template<typename O, typename... Is>
    struct Lambda : Expr<O> {
        tuple<VarPtr<Is>...> inputs;
        ExprPtr<O> output;

        Lambda(ExprPtr<O> output, VarPtr<Is>... inputs) :
            Lambda<O, Is...>{ output, move(make_tuple(inputs...)) }
        {}
        Lambda(ExprPtr<O> output, tuple<VarPtr<Is>...> inputs) :
            output(output), inputs(move(inputs))
        {}
    }; // struct Lambda
    template<typename O, typename... Is>
    using LambdaPtr = shared_ptr<Lambda<O, Is...>>;

} // namespace till

#endif // TILL_EXPR