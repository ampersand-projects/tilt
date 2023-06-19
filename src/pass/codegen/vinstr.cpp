#include "tilt/pass/codegen/vinstr.h"

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
    return reg->data + (((t / reg->dur) & reg->mask) * bytes);
}

region_t* make_region(region_t* out_reg, region_t* in_reg, ts_t st, ts_t et)
{
    out_reg->st = st;
    out_reg->et = et;
    out_reg->dur = in_reg->dur;
    out_reg->mask = in_reg->mask;
    out_reg->data = in_reg->data;
    out_reg->bitfield = in_reg->bitfield;
    return out_reg;
}

region_t* init_region(region_t* reg, ts_t t, dur_t dur, uint32_t size, char* data, char* bitfield)
{
    reg->st = t;
    reg->et = t;
    reg->dur = dur;
    reg->mask = size - 1;
    reg->data = data;
    reg->bitfield = bitfield;
    return reg;
}

region_t* commit_data(region_t* reg, ts_t t, char bit)
{
    reg->et = t;
    reg->bitfield[t / reg->dur] = bit;
    return reg;
}

region_t* commit_null(region_t* reg, ts_t t)
{
    reg->et = t;
    return reg;
}

}  // extern "C"
}  // namespace tilt
