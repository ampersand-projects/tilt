#include "tilt/codegen/vinstr.h"
#include "tilt/codegen/llvmgen.h"

#include "llvm/Linker/Linker.h"

#include "easy/jit.h"

using namespace std::placeholders;

namespace tilt {
extern "C" {

uint32_t get_buf_size(idx_t len)
{
    uint32_t ring = 1;
    while (len) { len >>= 1; ring <<= 1; }
    return ring;
}

idx_t get_start_idx(region_t* reg) { return reg->si; }

idx_t get_end_idx(region_t* reg) { return reg->ei; }

ts_t get_start_time(region_t* reg) { return reg->st; }

ts_t get_end_time(region_t* reg) { return reg->et; }

int64_t get_ckpt(region_t* reg, ts_t t, idx_t i)
{
    auto civl = reg->tl[i & reg->mask];
    return (t <= civl.t) ? civl.t : (civl.t + civl.d);
}

idx_t advance(region_t* reg, idx_t i, ts_t t)
{
    while ((reg->tl[i & reg->mask].t + reg->tl[i & reg->mask].d) < t) { i++; }
    return i;
}

char* fetch(region_t* reg, ts_t t, idx_t i, uint32_t bytes)
{
    auto ivl = reg->tl[i & reg->mask];
    return (t <= ivl.t) ? nullptr : (reg->data + ((i & reg->mask) * bytes));
}

region_t* make_region(region_t* out_reg, region_t* in_reg, ts_t st, idx_t si, ts_t et, idx_t ei)
{
    out_reg->st = st;
    out_reg->si = si;
    out_reg->et = et;
    out_reg->ei = ei;
    out_reg->mask = in_reg->mask;
    out_reg->tl = in_reg->tl;
    out_reg->data = in_reg->data;

    return out_reg;
}

region_t* init_region(region_t* reg, ts_t t, uint32_t size, ival_t* tl, char* data)
{
    reg->st = t;
    reg->si = 0;
    reg->et = t;
    reg->ei = -1;
    reg->mask = size - 1;
    tl[0] = {t, 0};
    reg->tl = tl;
    reg->data = data;
    return reg;
}

region_t* commit_data(region_t* reg, ts_t t)
{
    auto last_ckpt = reg->et;
    reg->et = t;
    reg->ei++;

    reg->tl[reg->ei & reg->mask].t = last_ckpt;
    reg->tl[reg->ei & reg->mask].d = t - last_ckpt;

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
    REGISTER_VINSTR(*llmod(), llctx(), get_start_idx, _1);
    REGISTER_VINSTR(*llmod(), llctx(), get_end_idx, _1);
    REGISTER_VINSTR(*llmod(), llctx(), get_start_time, _1);
    REGISTER_VINSTR(*llmod(), llctx(), get_end_time, _1);
    REGISTER_VINSTR(*llmod(), llctx(), get_ckpt, _1, _2, _3);
    REGISTER_VINSTR(*llmod(), llctx(), advance, _1, _2, _3);
    REGISTER_VINSTR(*llmod(), llctx(), fetch, _1, _2, _3, _4);
    REGISTER_VINSTR(*llmod(), llctx(), make_region, _1, _2, _3, _4, _5, _6);
    REGISTER_VINSTR(*llmod(), llctx(), init_region, _1, _2, _3, _4);
    REGISTER_VINSTR(*llmod(), llctx(), commit_data, _1, _2);
    REGISTER_VINSTR(*llmod(), llctx(), commit_null, _1, _2);
}

}  // namespace tilt
