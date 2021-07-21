#include "tilt/codegen/vinstr.h"
#include "tilt/codegen/llvmgen.h"

#include "llvm/Linker/Linker.h"

#include "easy/jit.h"

using namespace std::placeholders;

namespace tilt {
extern "C" {

index_t* get_start_idx(region_t* reg) { return &reg->si; }

index_t* get_end_idx(region_t* reg) { return &reg->ei; }

int64_t get_time(index_t* idx) { return idx->t; }

uint32_t get_index(index_t* idx) { return idx->i; }

int64_t next_time(region_t* reg, index_t* idx)
{
    auto t = idx->t;
    auto civl = reg->tl[idx->i];
    auto cst = civl.t;
    auto cet = cst + civl.i;

    if (t < cet) {
        return (t < cst) ? cst : cet;
    } else {
        auto nivl = reg->tl[idx->i + 1];
        auto nst = nivl.t;
        auto net = nst + nivl.i;
        return (t < nst) ? nst : net;
    }
}

index_t* advance(region_t* reg, index_t* idx, int64_t t)
{
    auto i = idx->i;
    while ((reg->tl[i].t + reg->tl[i].i) < t) { i++; }
    idx->t = t;
    idx->i = i;
    return idx;
}

char* fetch(region_t* reg, index_t* idx, uint32_t size)
{
    auto ivl = reg->tl[idx->i];
    return (idx->t <= ivl.t) ? nullptr : (reg->data + (idx->i * size));
}

region_t* make_region(region_t* out_reg, region_t* in_reg, index_t* si, index_t* ei)
{
    out_reg->si = *si;
    out_reg->ei = *ei;
    out_reg->tl = in_reg->tl;
    out_reg->data = in_reg->data;

    return out_reg;
}

region_t* init_region(region_t* reg, uint64_t t, index_t* tl, char* data)
{
    reg->si.t = t;
    reg->si.i = 0;
    reg->ei.t = t;
    reg->ei.i = -1;
    reg->tl = tl;
    reg->data = data;
    return reg;
}

region_t* commit_data(region_t* reg, int64_t t)
{
    auto last_ckpt = reg->ei.t;
    reg->ei.t = t;
    reg->ei.i++;

    reg->tl[reg->ei.i].t = last_ckpt;
    reg->tl[reg->ei.i].i = t - last_ckpt;

    return reg;
}

region_t* commit_null(region_t* reg, int64_t t)
{
    reg->ei.t = t;
    return reg;
}

}  // extern "C"

void LLVMGen::register_vinstrs()
{
    llvm::Linker::linkModules(*llmod(), easy::get_module(llctx(), get_start_idx, _1));
    llvm::Linker::linkModules(*llmod(), easy::get_module(llctx(), get_end_idx, _1));
    llvm::Linker::linkModules(*llmod(), easy::get_module(llctx(), get_time, _1));
    llvm::Linker::linkModules(*llmod(), easy::get_module(llctx(), get_index, _1));
    llvm::Linker::linkModules(*llmod(), easy::get_module(llctx(), next_time, _1, _2));
    llvm::Linker::linkModules(*llmod(), easy::get_module(llctx(), advance, _1, _2, _3));
    llvm::Linker::linkModules(*llmod(), easy::get_module(llctx(), fetch, _1, _2, _3));
    llvm::Linker::linkModules(*llmod(), easy::get_module(llctx(), make_region, _1, _2, _3, _4));
    llvm::Linker::linkModules(*llmod(), easy::get_module(llctx(), init_region, _1, _2, _3, _4));
    llvm::Linker::linkModules(*llmod(), easy::get_module(llctx(), commit_data, _1, _2));
    llvm::Linker::linkModules(*llmod(), easy::get_module(llctx(), commit_null, _1, _2));
}

}  // namespace tilt
