#include "tilt/base/type.h"
#include "tilt/builder/tilder.h"

namespace tilt::tilder {

    Const _i8(int8_t v) { return _iconst(types::INT8, v); }
    Const _i16(int16_t v) { return _iconst(types::INT16, v); }
    Const _i32(int32_t v) { return _iconst(types::INT32, v); }
    Const _i64(int64_t v) { return _iconst(types::INT64, v); }
    Const _u8(uint8_t v) { return _uconst(types::UINT8, v); }
    Const _u16(uint16_t v) { return _uconst(types::UINT16, v); }
    Const _u32(uint32_t v) { return _uconst(types::UINT32, v); }
    Const _u64(uint64_t v) { return _uconst(types::UINT64, v); }
    Const _f32(float v) { return _fconst(types::FLOAT32, v); }
    Const _f64(double v) { return _fconst(types::FLOAT64, v); }
    Const _ch(char v) { return _cconst(v); }
    Const _ts(long v) { return _tconst(v); }

} // namespace tilt::tilder