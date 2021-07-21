#include "tilt/codegen/vinstr.h"
#include "tilt/codegen/llvmgen.h"

#include "llvm/Linker/Linker.h"

#include "easy/jit.h"

using namespace std::placeholders;

namespace tilt {
extern "C" {

idx_t get_start_idx(region_t* reg) { return reg->si; }

idx_t get_end_idx(region_t* reg) { return reg->ei; }

int64_t get_ckpt(region_t* reg, ts_t t, idx_t i)
{
    auto civl = reg->tl[i];
    return (t <= civl.t) ? civl.t : (civl.t + civl.d);
}

idx_t advance(region_t* reg, idx_t i, ts_t t)
{
    while ((reg->tl[i].t + reg->tl[i].d) < t) { i++; }
    return i;
}

char* fetch(region_t* reg, ts_t t, idx_t i, uint32_t size)
{
    auto ivl = reg->tl[i];
    return (t <= ivl.t) ? nullptr : (reg->data + (i * size));
}

region_t* make_region(region_t* out_reg, region_t* in_reg, ts_t st, idx_t si, ts_t et, idx_t ei)
{
    out_reg->st = st;
    out_reg->si = si;
    out_reg->et = et;
    out_reg->ei = ei;
    out_reg->tl = in_reg->tl;
    out_reg->data = in_reg->data;

    return out_reg;
}

region_t* init_region(region_t* reg, ts_t t, ival_t* tl, char* data)
{
    reg->st = t;
    reg->si = 0;
    reg->et = t;
    reg->ei = -1;
    reg->tl = tl;
    reg->data = data;
    return reg;
}

region_t* commit_data(region_t* reg, ts_t t)
{
    auto last_ckpt = reg->et;
    reg->et = t;
    reg->ei++;

    reg->tl[reg->ei].t = last_ckpt;
    reg->tl[reg->ei].d = t - last_ckpt;

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
    llvm::Linker::linkModules(*llmod(), easy::get_module(llctx(), get_start_idx, _1));
    llvm::Linker::linkModules(*llmod(), easy::get_module(llctx(), get_end_idx, _1));
    llvm::Linker::linkModules(*llmod(), easy::get_module(llctx(), get_ckpt, _1, _2, _3));
    llvm::Linker::linkModules(*llmod(), easy::get_module(llctx(), advance, _1, _2, _3));
    llvm::Linker::linkModules(*llmod(), easy::get_module(llctx(), fetch, _1, _2, _3, _4));
    llvm::Linker::linkModules(*llmod(), easy::get_module(llctx(), make_region, _1, _2, _3, _4, _5, _6));
    llvm::Linker::linkModules(*llmod(), easy::get_module(llctx(), init_region, _1, _2, _3, _4));
    llvm::Linker::linkModules(*llmod(), easy::get_module(llctx(), commit_data, _1, _2));
    llvm::Linker::linkModules(*llmod(), easy::get_module(llctx(), commit_null, _1, _2));
}

}  // namespace tilt