#ifndef TILT_TILDER
#define TILT_TILDER

#include "tilt/ir/expr.h"
#include "tilt/ir/lstream.h"
#include "tilt/ir/op.h"
#include "tilt/ir/loop.h"

namespace tilt::tilder {

    template<typename T>
    struct _tilder : public shared_ptr<T> {
        _tilder(shared_ptr<T>&& ptr) : shared_ptr<T>(move(ptr)) {}
    };

    #define REGISTER_TILDER(NAME, EXPR) \
        template<typename... Args> \
        struct NAME : public _tilder<EXPR> { \
            NAME(Args... args) : \
                _tilder<EXPR>(move(make_shared<EXPR>(forward<Args>(args)...))) \
            {} \
        };

    // Symbol
    REGISTER_TILDER(_sym, Symbol)

    // Arithmetic expressions
    REGISTER_TILDER(_add, Add)
    REGISTER_TILDER(_sub, Sub)
    REGISTER_TILDER(_max, Max)
    REGISTER_TILDER(_min, Min)
    REGISTER_TILDER(_lt, LessThan)
    REGISTER_TILDER(_lte, LessThanEqual)
    REGISTER_TILDER(_gt, GreaterThan)
    
    // Logical expressions
    REGISTER_TILDER(_true, True)
    REGISTER_TILDER(_false, False)
    REGISTER_TILDER(_exists, Exists)
    REGISTER_TILDER(_not, Not)
    REGISTER_TILDER(_eq, Equals)
    REGISTER_TILDER(_and, And)
    REGISTER_TILDER(_or, Or)
    
    // Constant expressions
    REGISTER_TILDER(_iconst, IConst)
    REGISTER_TILDER(_uconst, UConst)
    REGISTER_TILDER(_fconst, FConst)
    REGISTER_TILDER(_cconst, CConst)
    REGISTER_TILDER(_tconst, TConst)

    // LStream operations
    REGISTER_TILDER(_subls, SubLStream)
    REGISTER_TILDER(_elem, Element)
    REGISTER_TILDER(_op, OpNode)

    // Misc expressions
    REGISTER_TILDER(_call, Call)
    REGISTER_TILDER(_get, Get)
    REGISTER_TILDER(_new, New)
    REGISTER_TILDER(_sel, IfElse)
    REGISTER_TILDER(_now, Now)
    REGISTER_TILDER(_agg, AggNode)

    // Loop IR expressions
    REGISTER_TILDER(_time, Time)
    REGISTER_TILDER(_idx, Index)
    REGISTER_TILDER(_reg, Region)
    REGISTER_TILDER(_get_time, GetTime)
    REGISTER_TILDER(_get_idx, GetIndex)
    REGISTER_TILDER(_fetch, Fetch)
    REGISTER_TILDER(_load, Load)
    REGISTER_TILDER(_store, Store)
    REGISTER_TILDER(_adv, Advance)
    REGISTER_TILDER(_next_time, NextTime)
    REGISTER_TILDER(_get_start_idx, GetStartIdx)
    REGISTER_TILDER(_get_end_idx, GetEndIdx)
    REGISTER_TILDER(_commit_data, CommitData)
    REGISTER_TILDER(_commit_null, CommitNull)
    REGISTER_TILDER(_alloc_idx, AllocIndex)
    REGISTER_TILDER(_alloc_reg, AllocRegion)
    REGISTER_TILDER(_make_reg, MakeRegion)
    REGISTER_TILDER(_loop, Loop)

    #undef REGISTER_TILDER


    Const _i8(int8_t);
    Const _i16(int16_t);
    Const _i32(int32_t);
    Const _i64(int64_t);
    Const _u8(uint8_t);
    Const _u16(uint16_t);
    Const _u32(uint32_t);
    Const _u64(uint64_t);
    Const _f32(float);
    Const _f64(double);
    Const _ch(char);
    Const _ts(long);

    using _iter = Iter;
    using _pt = Point;
    using _win = Window;

} // namespace tilt::tilder

#endif // TILT_TILDER