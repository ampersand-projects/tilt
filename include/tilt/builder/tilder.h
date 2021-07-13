#ifndef TILT_TILDER
#define TILT_TILDER

#include "tilt/ir/expr.h"
#include "tilt/ir/lstream.h"

namespace tilt {

    /**
     * Arithmetic expressions
     */
    ValExprPtr _add(ExprPtr, ExprPtr);
    ValExprPtr _sub(ExprPtr, ExprPtr);
    ValExprPtr _max(ExprPtr, ExprPtr);
    ValExprPtr _min(ExprPtr, ExprPtr);

    /**
     * Logical expressions
     */
    PredPtr _true();
    PredPtr _false();
    PredPtr _exists(SymPtr);
    PredPtr _not(ExprPtr);
    PredPtr _eq(ExprPtr, ExprPtr);
    PredPtr _and(ExprPtr, ExprPtr);
    PredPtr _or(ExprPtr, ExprPtr);
    PredPtr _lt(ExprPtr, ExprPtr);
    PredPtr _lte(ExprPtr, ExprPtr);
    PredPtr _gt(ExprPtr, ExprPtr);

    /**
     * Constant expressions
     */
    ConstPtr _i8(int64_t);
    ConstPtr _i16(int64_t);
    ConstPtr _i32(int64_t);
    ConstPtr _i64(int64_t);
    ConstPtr _u8(int64_t);
    ConstPtr _u16(int64_t);
    ConstPtr _u32(int64_t);
    ConstPtr _u64(int64_t);
    ConstPtr _f32(double);
    ConstPtr _f64(double);
    ConstPtr _ch(char);
    ConstPtr _time(long);

    /**
     * Misc expressions
     */
    ExprPtr _call(FuncPtr, vector<ExprPtr>);
    ExprPtr _sel(ExprPtr, ExprPtr, ExprPtr);
    ExprPtr _now();

    /**
     * LStream operations
     */
    SymPtr _ls(string, DataType, Iter);
    SymPtr _ls(string, DataType);
    Point _pt(long);
    Window _win(long, long);
    ElemPtr _elem(SymPtr, Point);
    SubLSPtr _subls(SymPtr, Window);

} // namespace tilt

#endif // TILT_TILDER