#ifndef INCLUDE_TILT_BASE_CTYPE_H_
#define INCLUDE_TILT_BASE_CTYPE_H_

extern "C" {

typedef signed long int64_t;
typedef unsigned int uint32_t;

typedef int64_t ts_t;
typedef int64_t idx_t;
typedef uint32_t dur_t;

struct ival_t {
    ts_t t;
    dur_t d;
};

struct region_t {
    ts_t st;
    ts_t et;
    idx_t head;
    idx_t count;
    uint32_t mask;
    ival_t* tl;
    char* data;
};

}  // extern "C"

#endif  // INCLUDE_TILT_BASE_CTYPE_H_