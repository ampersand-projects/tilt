#include "tilt/base/type.h"
#include "tilt/builder/tilder.h"

namespace tilt::tilder {

_expr<Add> _expr_add(Expr a, Expr b) { return _add(a, b); }
_expr<Sub> _expr_sub(Expr a, Expr b) { return _sub(a, b); }
_expr<Mul> _expr_mul(Expr a, Expr b) { return _mul(a, b); }
_expr<Div> _expr_div(Expr a, Expr b) { return _div(a, b); }
_expr<Neg> _expr_neg(Expr a) { return _neg(a); }
_expr<Mod> _expr_mod(Expr a, Expr b) { return _mod(a, b); }
_expr<LessThan> _expr_lt(Expr a, Expr b) { return _lt(a, b); }
_expr<LessThanEqual> _expr_lte(Expr a, Expr b) { return _lte(a, b); }
_expr<GreaterThan> _expr_gt(Expr a, Expr b) { return _gt(a, b); }
_expr<GreaterThanEqual> _expr_gte(Expr a, Expr b) { return _gte(a, b); }
_expr<Equals> _expr_eq(Expr a, Expr b) { return _eq(a, b); }
_expr<Not> _expr_not(Expr a) { return _not(a); }
_expr<And> _expr_and(Expr a, Expr b) { return _and(a, b); }
_expr<Or> _expr_or(Expr a, Expr b) { return _or(a, b); }
_expr<Get> _expr_get(Expr a, size_t n) { return _get(a, n); }
_expr<Element> _expr_elem(Sym a, Point pt) { return _elem(a, pt); }
_expr<SubLStream> _expr_subls(Sym a, Window win) { return _subls(a, win); }

_expr<ConstNode> _i8(int8_t v) { return _const(BaseType::INT8, v); }
_expr<ConstNode> _i16(int16_t v) { return _const(BaseType::INT16, v); }
_expr<ConstNode> _i32(int32_t v) { return _const(BaseType::INT32, v); }
_expr<ConstNode> _i64(int64_t v) { return _const(BaseType::INT64, v); }
_expr<ConstNode> _u8(uint8_t v) { return _const(BaseType::UINT8, v); }
_expr<ConstNode> _u16(uint16_t v) { return _const(BaseType::UINT16, v); }
_expr<ConstNode> _u32(uint32_t v) { return _const(BaseType::UINT32, v); }
_expr<ConstNode> _u64(uint64_t v) { return _const(BaseType::UINT64, v); }
_expr<ConstNode> _f32(float v) { return _const(BaseType::FLOAT32, v); }
_expr<ConstNode> _f64(double v) { return _const(BaseType::FLOAT64, v); }
_expr<ConstNode> _ts(int64_t v) { return _const(BaseType::TIME, v); }
_expr<ConstNode> _idx(idx_t v) { return _const(BaseType::INDEX, v); }
_expr<ConstNode> _true() { return _const(BaseType::BOOL, 1); }
_expr<ConstNode> _false() { return _const(BaseType::BOOL, 0); }

}  // namespace tilt::tilder
