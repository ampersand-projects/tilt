#ifndef TILT_VINSTR
#define TILT_VINSTR

#include "tilt/base/type.h"

#define TILT_VINSTR_ATTR __attribute__((always_inline))

namespace tilt { extern "C" {

    TILT_VINSTR_ATTR
    index_t* get_start_idx(region_t* reg)
    {
        return &reg->si;
    }

    TILT_VINSTR_ATTR
    index_t* get_end_idx(region_t* reg)
    {
        return &reg->ei;
    }

    TILT_VINSTR_ATTR
    long get_time(index_t* idx)
    {
        return idx->t;
    }

    TILT_VINSTR_ATTR
    unsigned int get_index(index_t* idx)
    {
        return idx->i;
    }

    TILT_VINSTR_ATTR
    long next_time(region_t* reg, index_t* idx)
    {
        auto t = idx->t;
        auto civl = reg->tl[idx->i];
        auto cst = civl.t;
        auto cet = cst + civl.i;

        if (t < cet) { return (t < cst) ? cst : cet; }
        else {
            auto nivl = reg->tl[idx->i + 1];
            auto nst = nivl.t;
            auto net = nst + nivl.i;
            return (t < nst) ? nst : net;
        }
    }

    TILT_VINSTR_ATTR
    index_t* advance(region_t* reg, index_t* idx, long t)
    {
        auto i = idx->i;
        while ((reg->tl[i].t + reg->tl[i].i) < t) { i++; }
        idx->t = t;
        idx->i = i;
        return idx;
    }

    TILT_VINSTR_ATTR
    char* fetch(region_t* reg, index_t* idx, unsigned int size)
    {
        auto ivl = reg->tl[idx->i];
        return (idx->t <= ivl.t) ? nullptr : (reg->data + (idx->i * size));
    }

    TILT_VINSTR_ATTR
    region_t* make_region(region_t* out_reg, region_t* in_reg, index_t* si, index_t* ei)
    {
        out_reg->si = *si;
        out_reg->ei = *ei;
        out_reg->tl = in_reg->tl;
        out_reg->data = in_reg->data;

        return out_reg;
    }

    TILT_VINSTR_ATTR
    region_t* init_region(region_t* reg, long t, index_t* tl, char* data)
    {
        reg->si.t = t;
        reg->si.i = 0;
        reg->ei.t = t;
        reg->ei.i = -1;
        reg->tl = tl;
        reg->data = data;
        return reg;
    }

    TILT_VINSTR_ATTR
    region_t* commit_data(region_t* reg, long t)
    {
        auto last_ckpt = reg->ei.t;
        reg->ei.t = t;
        reg->ei.i++;

        reg->tl[reg->ei.i].t = last_ckpt;
        reg->tl[reg->ei.i].i = t - last_ckpt;

        return reg;
    }

    TILT_VINSTR_ATTR
    region_t* commit_null(region_t* reg, long t)
    {
        reg->ei.t = t;
        return reg;
    }

} } // namespace tilt

#endif // TILT_VINSTR