#ifndef PYTHON_REGION_INCLUDE_CVTR_H_
#define PYTHON_REGION_INCLUDE_CVTR_H_

#include <memory>

#include "tilt/base/type.h"

class Cvtr {
public:
    static uint32_t dt_to_bytes(tilt::DataType dt);
    static uint32_t dt_to_bytes(std::shared_ptr<tilt::DataType> dt);
};

#endif  // PYTHON_REGION_INCLUDE_CVTR_H_

