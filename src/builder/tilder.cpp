#include "tilt/builder/tilder.h"

namespace tilt {

    ValExprPtr _add(ExprPtr a, ExprPtr b) { return make_shared<Add>(a, b); }
    ValExprPtr _sub(ExprPtr a, ExprPtr b) { return make_shared<Sub>(a, b); }
    ValExprPtr _max(ExprPtr a, ExprPtr b) { return make_shared<Max>(a, b); }
    ValExprPtr _min(ExprPtr a, ExprPtr b) { return make_shared<Min>(a, b); }
    PredPtr _true() { return make_shared<True>(); }
    PredPtr _false() { return make_shared<False>(); }
    PredPtr _exists(SymPtr a) { return make_shared<Exists>(a); }
    PredPtr _not(ExprPtr a) { return make_shared<Not>(a); }
    PredPtr _eq(ExprPtr a, ExprPtr b) { return make_shared<Equals>(a, b); }
    PredPtr _and(ExprPtr a, ExprPtr b) { return make_shared<And>(a, b); }
    PredPtr _or(ExprPtr a, ExprPtr b) { return make_shared<Or>(a, b); }
    PredPtr _lt(ExprPtr a, ExprPtr b) { return make_shared<LessThan>(a, b); }
    PredPtr _lte(ExprPtr a, ExprPtr b) { return make_shared<LessThanEqual>(a, b); }
    PredPtr _gt(ExprPtr a, ExprPtr b) { return make_shared<GreaterThan>(a, b); }
    ConstPtr _i8(int64_t a) { return make_shared<IConst>(types::INT8, a); }
    ConstPtr _i16(int64_t a) { return make_shared<IConst>(types::INT16, a); }
    ConstPtr _i32(int64_t a) { return make_shared<IConst>(types::INT32, a); }
    ConstPtr _i64(int64_t a) { return make_shared<IConst>(types::INT64, a); }
    ConstPtr _u8(int64_t a) { return make_shared<IConst>(types::UINT8, a); }
    ConstPtr _u16(int64_t a) { return make_shared<IConst>(types::UINT16, a); }
    ConstPtr _u32(int64_t a) { return make_shared<IConst>(types::UINT32, a); }
    ConstPtr _u64(int64_t a) { return make_shared<IConst>(types::UINT64, a); }
    ConstPtr _f32(double a) { return make_shared<IConst>(types::FLOAT32, a); }
    ConstPtr _f64(double a) { return make_shared<IConst>(types::FLOAT64, a); }
    ConstPtr _ch(char a) { return make_shared<IConst>(types::CHAR, a); }
    ConstPtr _time(long a) { return make_shared<IConst>(types::TIME, a); }
    ExprPtr _call(FuncPtr fn, vector<ExprPtr> args) { return make_shared<Call>(fn, args); }
    ExprPtr _sel(ExprPtr c, ExprPtr a, ExprPtr b) { return make_shared<IfElse>(c, a, b); }
    ExprPtr _now() { return make_shared<Now>(); }
    SymPtr _ls(string n, DataType d, Iter i) { return make_shared<Symbol>(n, Type(d, i)); }
    SymPtr _ls(string n, DataType d) { return _ls(n, d, FreeIter(n)); }
    Point _pt(long o) { return Point(o); }
    Window _win(long s, long e) { return Window(s, e); }
    ElemPtr _elem(SymPtr s, Point p) { return make_shared<Element>(s, p); }
    SubLSPtr _subls(SymPtr s, Window w) { return make_shared<SubLStream>(s, w); }

} // namespace tilt