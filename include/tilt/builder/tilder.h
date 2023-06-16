#ifndef INCLUDE_TILT_BUILDER_TILDER_H_
#define INCLUDE_TILT_BUILDER_TILDER_H_

#include <memory>
#include <utility>
#include <string>

#include "tilt/ir/expr.h"
#include "tilt/ir/lstream.h"
#include "tilt/ir/op.h"
#include "tilt/ir/loop.h"

namespace tilt::tilder {

template<typename T>
struct _expr;

_expr<Add> _expr_add(Expr, Expr);
_expr<Sub> _expr_sub(Expr, Expr);
_expr<Mul> _expr_mul(Expr, Expr);
_expr<Div> _expr_div(Expr, Expr);
_expr<Neg> _expr_neg(Expr);
_expr<Mod> _expr_mod(Expr, Expr);
_expr<LessThan> _expr_lt(Expr, Expr);
_expr<LessThanEqual> _expr_lte(Expr, Expr);
_expr<GreaterThan> _expr_gt(Expr, Expr);
_expr<GreaterThanEqual> _expr_gte(Expr, Expr);
_expr<Equals> _expr_eq(Expr, Expr);
_expr<Not> _expr_not(Expr);
_expr<And> _expr_and(Expr, Expr);
_expr<Or> _expr_or(Expr, Expr);
_expr<Get> _expr_get(Expr, size_t);
_expr<Element> _expr_elem(Sym, Point);
_expr<SubLStream> _expr_subls(Sym, Window);

template<typename T>
struct _expr : public shared_ptr<T> {
    explicit _expr(shared_ptr<T>&& ptr) : shared_ptr<T>(std::move(ptr)) {}

    _expr<Add> operator+(Expr o) const { return _expr_add(*this, o); }
    _expr<Sub> operator-(Expr o) const { return _expr_sub(*this, o); }
    _expr<Mul> operator*(Expr o) const { return _expr_mul(*this, o); }
    _expr<Div> operator/(Expr o) const { return _expr_div(*this, o); }
    _expr<Neg> operator-() const { return _expr_neg(*this); }
    _expr<Mod> operator%(Expr o) const { return _expr_mod(*this, o); }
    _expr<LessThan> operator<(Expr o) const { return _expr_lt(*this, o); }
    _expr<LessThanEqual> operator<=(Expr o) const { return _expr_lte(*this, o); }
    _expr<GreaterThan> operator>(Expr o) const { return _expr_gt(*this, o); }
    _expr<GreaterThanEqual> operator>=(Expr o) const { return _expr_gte(*this, o); }
    _expr<Equals> operator==(Expr o) const { return _expr_eq(*this, o); }
    _expr<Not> operator!() const { return _expr_not(*this); }
    _expr<And> operator&&(Expr o) const { return _expr_and(*this, o); }
    _expr<Or> operator||(Expr o) const { return _expr_or(*this, o); }
    _expr<Get> operator<<(size_t n) const { return _expr_get(*this, n); }
};

// Symbol
struct _sym : public _expr<Symbol> {
    _sym(string name, Type type) : _expr<Symbol>(make_shared<Symbol>(name, type)) {}
    _sym(string name, Expr expr) : _expr<Symbol>(make_shared<Symbol>(name, expr)) {}
    explicit _sym(const Symbol& symbol) : _sym(symbol.name, symbol.type) {}

    _expr<Element> operator[](Point pt) const { return _expr_elem(*this, pt); }
    _expr<SubLStream> operator[](Window win) const { return _expr_subls(*this, win); }
};

struct _out : public _expr<Out> {
    explicit _out(DataType dtype, Iter iter) : _expr<Out>(make_shared<Out>(dtype, iter)) {}
    explicit _out(const Out& out) : _out(out.type.dtype, out.type.iter) {}

    _expr<Element> operator[](Point pt) const { return _expr_elem(*this, pt); }
    _expr<SubLStream> operator[](Window win) const { return _expr_subls(*this, win); }
};

struct _beat : public _expr<Beat> {
    explicit _beat(Iter iter) : _expr<Beat>(make_shared<Beat>(iter)) {}
    explicit _beat(const Beat& beat) : _beat(beat.type.iter) {}

    _expr<Element> operator[](Point pt) const { return _expr_elem(*this, pt); }
    _expr<SubLStream> operator[](Window win) const { return _expr_subls(*this, win); }
};

#define REGISTER_EXPR(NAME, EXPR) \
    template<typename... Args> \
    struct NAME : public _expr<EXPR> { \
        explicit NAME(Args... args) : \
            _expr<EXPR>(std::move(make_shared<EXPR>(std::forward<Args>(args)...))) \
        {} \
    };

// Arithmetic expressions
REGISTER_EXPR(_add, Add)
REGISTER_EXPR(_sub, Sub)
REGISTER_EXPR(_mul, Mul)
REGISTER_EXPR(_div, Div)
REGISTER_EXPR(_max, Max)
REGISTER_EXPR(_min, Min)
REGISTER_EXPR(_abs, Abs)
REGISTER_EXPR(_neg, Neg)
REGISTER_EXPR(_mod, Mod)
REGISTER_EXPR(_sqrt, Sqrt)
REGISTER_EXPR(_pow, Pow)
REGISTER_EXPR(_ceil, Ceil)
REGISTER_EXPR(_floor, Floor)
REGISTER_EXPR(_lt, LessThan)
REGISTER_EXPR(_lte, LessThanEqual)
REGISTER_EXPR(_gt, GreaterThan)
REGISTER_EXPR(_gte, GreaterThanEqual)
REGISTER_EXPR(_eq, Equals)

// Logical expressions
REGISTER_EXPR(_exists, Exists)
REGISTER_EXPR(_not, Not)
REGISTER_EXPR(_and, And)
REGISTER_EXPR(_or, Or)

// Constant expressions
REGISTER_EXPR(_const, ConstNode)

// LStream operations
REGISTER_EXPR(_subls, SubLStream)
REGISTER_EXPR(_elem, Element)
REGISTER_EXPR(_op, OpNode)

// Misc expressions
REGISTER_EXPR(_call, Call)
REGISTER_EXPR(_read, Read)
REGISTER_EXPR(_get, Get)
REGISTER_EXPR(_new, New)
REGISTER_EXPR(_ifelse, IfElse)
REGISTER_EXPR(_sel, Select)
REGISTER_EXPR(_red, Reduce)
REGISTER_EXPR(_cast, Cast)

// Loop IR expressions
REGISTER_EXPR(_time, TimeNode)
REGISTER_EXPR(_fetch, Fetch)
REGISTER_EXPR(_write, Write)
REGISTER_EXPR(_get_ckpt, GetCkpt)
REGISTER_EXPR(_get_start_time, GetStartTime)
REGISTER_EXPR(_get_end_time, GetEndTime)
REGISTER_EXPR(_commit_data, CommitData)
REGISTER_EXPR(_commit_null, CommitNull)
REGISTER_EXPR(_alloc_reg, AllocRegion)
REGISTER_EXPR(_make_reg, MakeRegion)
REGISTER_EXPR(_loop, LoopNode)

#undef REGISTER_EXPR


_expr<ConstNode> _i8(int8_t);
_expr<ConstNode> _i16(int16_t);
_expr<ConstNode> _i32(int32_t);
_expr<ConstNode> _i64(int64_t);
_expr<ConstNode> _u8(uint8_t);
_expr<ConstNode> _u16(uint16_t);
_expr<ConstNode> _u32(uint32_t);
_expr<ConstNode> _u64(uint64_t);
_expr<ConstNode> _f32(float);
_expr<ConstNode> _f64(double);
_expr<ConstNode> _ch(char);
_expr<ConstNode> _ts(ts_t);
_expr<ConstNode> _true();
_expr<ConstNode> _false();

using _iter = Iter;
using _pt = Point;
using _win = Window;

}  // namespace tilt::tilder

#endif  // INCLUDE_TILT_BUILDER_TILDER_H_
