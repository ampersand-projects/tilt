#ifndef INCLUDE_TILT_PYTHON_REGION_CVTR_H_
#define INCLUDE_TILT_PYTHON_REGION_CVTR_H_

#include "tilt/base/type.h"

class Cvtr {
public:
    static uint32_t dt_to_bytes(tilt::DataType dt);
    static uint32_t dt_to_bytes(std::shared_ptr<tilt::DataType> dt);
};

#endif // INCLUDE_TILT_PYTHON_REGION_CVTR_H_
