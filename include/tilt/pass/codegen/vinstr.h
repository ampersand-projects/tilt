#ifndef INCLUDE_TILT_PASS_CODEGEN_VINSTR_H_
#define INCLUDE_TILT_PASS_CODEGEN_VINSTR_H_

#include "tilt/base/ctype.h"

#define TILT_VINSTR_ATTR __attribute__((always_inline))

namespace tilt {
extern "C" {

// !! Make sure to add new vinstr to this array,
// !! otherwise they will not be accessible in LLVM
static const char* vinstr_names[] = {
    "get_buf_size",
    "get_start_idx",
    "get_end_idx",
    "get_start_time",
    "get_end_time",
    "get_ckpt",
    "advance",
    "fetch",
    "make_region",
    "init_region",
    "commit_data",
    "commit_null"
};

__attribute__((always_inline)) uint32_t get_buf_size(idx_t);
__attribute__((always_inline)) idx_t get_start_idx(region_t*);
__attribute__((always_inline)) idx_t get_end_idx(region_t*);
__attribute__((always_inline)) ts_t get_start_time(region_t*);
__attribute__((always_inline)) ts_t get_end_time(region_t*);
__attribute__((always_inline)) ts_t get_ckpt(region_t*, ts_t, idx_t);
__attribute__((always_inline)) idx_t advance(region_t*, idx_t, ts_t);
__attribute__((always_inline)) char* fetch(region_t*, ts_t, idx_t, uint32_t);
__attribute__((always_inline)) region_t* make_region(region_t*, region_t*, ts_t, idx_t, ts_t, idx_t);
__attribute__((always_inline)) region_t* init_region(region_t*, ts_t, uint32_t, ival_t*, char*);
__attribute__((always_inline)) region_t* commit_data(region_t*, ts_t);
__attribute__((always_inline)) region_t* commit_null(region_t*, ts_t);

}  // extern "C"
}  // namespace tilt

#endif  // INCLUDE_TILT_PASS_CODEGEN_VINSTR_H_
