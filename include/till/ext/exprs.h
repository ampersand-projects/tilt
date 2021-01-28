#ifndef TILL_EXPRS
#define TILL_EXPRS

#include "till/core/expr.h"

using namespace std;

namespace till {

    template<typename O, typename I>
    struct UnaryExpr : public Expr<O> {
        ExprPtr<I> input;
        UnaryExpr(ExprPtr<I> input) : input(input) {}
    }; // struct UnaryExpr

    template<typename O, typename L, typename R>
    struct BinaryExpr : public Expr<O> {
        ExprPtr<L> left;
        ExprPtr<R> right;
        BinaryExpr(ExprPtr<L> left, ExprPtr<R> right) :
            left(left), right(right)
        {}
    }; // struct UnaryExpr

    template<typename T>
    struct Add : public UnaryExpr<T, T> {
        ConstPtr<T> val;
        Add(ExprPtr<T> input, ConstPtr<T> val) :
            UnaryExpr<T, T>{ input }, val(val)
        {}
    }; // struct Add

} // namespace till

#endif // TILL_EXPRS