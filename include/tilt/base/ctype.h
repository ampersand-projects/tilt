#ifndef INCLUDE_TILT_BASE_CTYPE_H_
#define INCLUDE_TILT_BASE_CTYPE_H_

#include <cstdint>

typedef int64_t ts_t;
typedef int64_t idx_t;
typedef uint32_t dur_t;

extern "C" {

struct ival_t {
    ts_t t;
    dur_t d;
};

struct region_t {
    ts_t st;
    ts_t et;
    uint32_t mask;
    char* data;
};

}  // extern "C"

#endif  // INCLUDE_TILT_BASE_CTYPE_H_
