#include "tilt/base/type.h"
#include "tilt/builder/tilder.h"

namespace tilt::tilder {

Const _i8(int8_t v) { return _const(BaseType::INT8, v); }
Const _i16(int16_t v) { return _const(BaseType::INT16, v); }
Const _i32(int32_t v) { return _const(BaseType::INT32, v); }
Const _i64(int64_t v) { return _const(BaseType::INT64, v); }
Const _u8(uint8_t v) { return _const(BaseType::UINT8, v); }
Const _u16(uint16_t v) { return _const(BaseType::UINT16, v); }
Const _u32(uint32_t v) { return _const(BaseType::UINT32, v); }
Const _u64(uint64_t v) { return _const(BaseType::UINT64, v); }
Const _f32(float v) { return _const(BaseType::FLOAT32, v); }
Const _f64(double v) { return _const(BaseType::FLOAT64, v); }
Const _ch(char v) { return _const(BaseType::CHAR, v); }
Const _ts(int64_t v) { return _const(BaseType::TIME, v); }
Const _true() { return _const(BaseType::BOOL, 1); }
Const _false() { return _const(BaseType::BOOL, 0); }

}  // namespace tilt::tilder
