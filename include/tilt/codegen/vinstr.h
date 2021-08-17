#ifndef INCLUDE_TILT_CODEGEN_VINSTR_H_
#define INCLUDE_TILT_CODEGEN_VINSTR_H_

#include "tilt/base/type.h"

#define TILT_VINSTR_ATTR __attribute__((always_inline))

#define REGISTER_VINSTR(dest, ctx, vinst_name, ...) \
    llvm::Linker::linkModules(dest, easy::get_module(ctx, vinst_name, __VA_ARGS__)); \
    (dest).getFunction(#vinst_name)->setLinkage(llvm::Function::InternalLinkage);

namespace tilt {
extern "C" {

TILT_VINSTR_ATTR idx_t get_start_idx(region_t*);
TILT_VINSTR_ATTR idx_t get_end_idx(region_t*);
TILT_VINSTR_ATTR ts_t get_ckpt(region_t*, ts_t, idx_t);
TILT_VINSTR_ATTR idx_t advance(region_t*, idx_t, ts_t);
TILT_VINSTR_ATTR char* fetch(region_t*, ts_t, idx_t, uint32_t);
TILT_VINSTR_ATTR region_t* make_region(region_t*, region_t*, ts_t, idx_t, ts_t, idx_t);
TILT_VINSTR_ATTR region_t* init_region(region_t*, ts_t, ival_t*, char*);
TILT_VINSTR_ATTR region_t* commit_data(region_t*, ts_t);
TILT_VINSTR_ATTR region_t* commit_null(region_t*, ts_t);

}  // extern "C"

const std::vector<std::string> vinst_names = {"get_start_idx", "get_end_idx", "get_ckpt", "advance", "fetch",
                                             "make_region", "init_region", "commit_data", "commit_null"};

}  // namespace tilt

#endif  // INCLUDE_TILT_CODEGEN_VINSTR_H_
