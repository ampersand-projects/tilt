#include "cvtr.h"

#include <memory>

#include "tilt/base/type.h"

using namespace std;
using namespace tilt;

uint32_t Cvtr::dt_to_bytes(DataType dt)
{
    switch (dt.btype) {
        case BaseType::INT8: return 1;
        case BaseType::INT16: return 2;
        case BaseType::INT32: return 4;
        case BaseType::INT64: return 8;
        case BaseType::UINT8: return 1;
        case BaseType::UINT16: return 2;
        case BaseType::UINT32: return 4;
        case BaseType::UINT64: return 8;
        case BaseType::FLOAT32: return 4;
        case BaseType::FLOAT64: return 8;
        case BaseType::STRUCT: {
            uint32_t res = 0;
            for (const auto& dtype : dt.dtypes) {
                res += dt_to_bytes(dtype);
            }
            return res;
        }
        default: throw runtime_error("Invalid type");
    }
}

uint32_t Cvtr::dt_to_bytes(shared_ptr<DataType> dt)
{
    return dt_to_bytes(*dt);
}
