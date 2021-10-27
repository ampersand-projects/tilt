#include "tilt/pass/codegen/vinstr.h"
#include "tilt/pass/codegen/llvmgen.h"

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

idx_t get_start_idx(region_t* reg)
{
    auto size = reg->mask + 1;
    auto count = (reg->count < size) ? reg->count : size;
    return reg->head - count + 1;
}

idx_t get_end_idx(region_t* reg) { return reg->head; }

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
    out_reg->et = et;
    out_reg->head = ei;
    out_reg->count = ei - si + 1;
    out_reg->mask = in_reg->mask;
    out_reg->tl = in_reg->tl;
    out_reg->data = in_reg->data;

    return out_reg;
}

region_t* init_region(region_t* reg, ts_t t, uint32_t size, ival_t* tl, char* data)
{
    reg->st = t;
    reg->et = t;
    reg->head = -1;
    reg->count = 0;
    reg->mask = size - 1;
    reg->tl = tl;
    reg->data = data;
    commit_null(reg, t);
    return reg;
}

region_t* commit_data(region_t* reg, ts_t t)
{
    auto last_ckpt = reg->et;
    reg->et = t;
    reg->head++;
    reg->count++;

    reg->tl[reg->head & reg->mask].t = last_ckpt;
    reg->tl[reg->head & reg->mask].d = t - last_ckpt;

    return reg;
}

region_t* commit_null(region_t* reg, ts_t t)
{
    reg->et = t;
    reg->tl[(reg->head + 1) & reg->mask].t = t;
    reg->tl[(reg->head + 1) & reg->mask].d = 0;
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
