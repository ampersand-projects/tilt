#ifndef TILT_VINSTR
#define TILT_VINSTR

#include "tilt/base/type.h"

namespace tilt { extern "C" {

    index_t* get_start_idx(region_t* reg)
    {
        return &reg->si;
    }

    index_t* get_end_idx(region_t* reg)
    {
        return &reg->ei;
    }

    long get_time(index_t* idx)
    {
        return idx->t;
    }

    long next_time(region_t* reg, index_t* idx)
    {
        auto e = reg->tl[idx->i];
        auto et = e.t;
        auto st = e.t - e.i;
        return (idx->t < st) ? st : et;
    }

    index_t* advance(region_t* reg, index_t* idx, long t)
    {
        auto i = idx->i;
        while (reg->tl[i].t < t) { i++; }
        idx->t = t;
        idx->i = i;
        return idx;
    }

    char* fetch(region_t* reg, index_t* idx, unsigned int size)
    {
        return reg->data + (idx->i * size);
    }

    region_t* make_region(region_t* out_reg, region_t* in_reg, index_t* si, index_t* ei)
    {
        out_reg->si = *si;
        out_reg->ei = *ei;
        out_reg->tl = in_reg->tl;
        out_reg->data = in_reg->data;

        return out_reg;
    }

    region_t* commit_data(region_t* reg, long t)
    {
        auto et = reg->ei.t;
        auto dur = t - et;
        auto i = reg->ei.i + 1;

        reg->tl[i].t = t;
        reg->tl[i].i = dur;

        reg->ei.t = t;
        reg->ei.i = i;

        return reg;
    }

    region_t* commit_null(region_t* reg, long t)
    {
        reg->ei.t = t;
        return reg;
    }

} } // namespace tilt

#endif // TILT_VINSTR