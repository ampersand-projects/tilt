#ifndef INCLUDE_TILT_PASS_CODEGEN_LLVMTYPE_H_
#define INCLUDE_TILT_PASS_CODEGEN_LLVMTYPE_H_

#include <vector>

#include "tilt/base/type.h"
#include "tilt/ir/expr.h"

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Type.h"

using namespace std;

namespace tilt {

struct StructPaddingInfo {
    uint64_t total_bytes;
    vector<uint64_t> offsets;
};

class LLVMTypeGen {
public:
    explicit LLVMTypeGen(llvm::LLVMContext* llctx) :
        _llctx(llctx)
    {}

    llvm::Type* lltype(const DataType&);
    static StructPaddingInfo getStructPadding(const DataType&);
private:
    llvm::LLVMContext& llctx() { return *_llctx; }
    llvm::LLVMContext* _llctx;
};

}  // namespace tilt

#endif  // INCLUDE_TILT_PASS_CODEGEN_LLVMTYPE_H_
