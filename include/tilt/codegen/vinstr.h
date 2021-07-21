#ifndef INCLUDE_TILT_CODEGEN_VINSTR_H_
#define INCLUDE_TILT_CODEGEN_VINSTR_H_

#include "tilt/base/type.h"

#define TILT_VINSTR_ATTR __attribute__((always_inline))

namespace tilt {
extern "C" {

TILT_VINSTR_ATTR index_t* get_start_idx(region_t*);
TILT_VINSTR_ATTR index_t* get_end_idx(region_t*);
TILT_VINSTR_ATTR int64_t get_time(index_t*);
TILT_VINSTR_ATTR uint32_t get_index(index_t*);
TILT_VINSTR_ATTR int64_t next_time(region_t*, index_t*);
TILT_VINSTR_ATTR index_t* advance(region_t*, index_t*, int64_t);
TILT_VINSTR_ATTR char* fetch(region_t*, index_t*, uint32_t);
TILT_VINSTR_ATTR region_t* make_region(region_t*, region_t*, index_t*, index_t*);
TILT_VINSTR_ATTR region_t* init_region(region_t*, uint64_t, index_t*, char*);
TILT_VINSTR_ATTR region_t* commit_data(region_t*, int64_t);
TILT_VINSTR_ATTR region_t* commit_null(region_t*, int64_t);

}  // extern "C"
}  // namespace tilt

#endif  // INCLUDE_TILT_CODEGEN_VINSTR_H_
