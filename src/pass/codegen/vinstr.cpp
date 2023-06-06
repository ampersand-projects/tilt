#include "tilt/pass/codegen/vinstr.h"
#include "tilt/pass/codegen/llvmgen.h"

#include "llvm/Linker/Linker.h"

#include "easy/jit.h"

using namespace std::placeholders;

namespace tilt {
extern "C" {

uint32_t get_buf_size(ts_t len)
{
    uint32_t ring = 1;
    while (len) { len >>= 1; ring <<= 1; }
    return ring;
}

ts_t get_start_time(region_t* reg) { return reg->st; }

ts_t get_end_time(region_t* reg) { return reg->et; }

int64_t get_ckpt(region_t* reg, ts_t t) { return t; }

char* fetch(region_t* reg, ts_t t, uint32_t bytes)
{
    return reg->data + ((t & reg->mask) * bytes);
}

region_t* make_region(region_t* out_reg, region_t* in_reg, ts_t st, ts_t et)
{
    out_reg->st = st;
    out_reg->et = et;
    out_reg->mask = in_reg->mask;
    out_reg->data = in_reg->data;

    return out_reg;
}

region_t* init_region(region_t* reg, ts_t t, uint32_t size, char* data)
{
    reg->st = t;
    reg->et = t;
    reg->mask = size - 1;
    reg->data = data;
    commit_null(reg, t);
    return reg;
}

region_t* commit_data(region_t* reg, ts_t t)
{
    reg->et = t;
    return reg;
}

region_t* commit_null(region_t* reg, ts_t t)
{
    reg->et = t;
    return reg;
}

}  // extern "C"

void LLVMGen::register_vinstrs()
{
    REGISTER_VINSTR(*llmod(), llctx(), get_buf_size, _1);
    REGISTER_VINSTR(*llmod(), llctx(), get_start_time, _1);
    REGISTER_VINSTR(*llmod(), llctx(), get_end_time, _1);
    REGISTER_VINSTR(*llmod(), llctx(), get_ckpt, _1, _2);
    REGISTER_VINSTR(*llmod(), llctx(), fetch, _1, _2, _3);
    REGISTER_VINSTR(*llmod(), llctx(), make_region, _1, _2, _3, _4);
    REGISTER_VINSTR(*llmod(), llctx(), init_region, _1, _2, _3, _4);
    REGISTER_VINSTR(*llmod(), llctx(), commit_data, _1, _2);
    REGISTER_VINSTR(*llmod(), llctx(), commit_null, _1, _2);
}

}  // namespace tilt
