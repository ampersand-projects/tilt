#include <vector>
#include <memory>
#include <stdexcept>

#include "tilt/pass/codegen/llvmtype.h"
#include "tilt/base/type.h"

#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/DataLayout.h"

using namespace std;
using namespace tilt;
using namespace llvm;

llvm::Type* LLVMTypeGen::lltype(const DataType& dtype)
{
    switch (dtype.btype) {
        case BaseType::BOOL:
            return llvm::Type::getInt1Ty(llctx());
        case BaseType::INT8:
        case BaseType::UINT8:
            return llvm::Type::getInt8Ty(llctx());
        case BaseType::INT16:
        case BaseType::UINT16:
            return llvm::Type::getInt16Ty(llctx());
        case BaseType::INT32:
        case BaseType::UINT32:
            return llvm::Type::getInt32Ty(llctx());
        case BaseType::INT64:
        case BaseType::UINT64:
            return llvm::Type::getInt64Ty(llctx());
        case BaseType::FLOAT32:
            return llvm::Type::getFloatTy(llctx());
        case BaseType::FLOAT64:
            return llvm::Type::getDoubleTy(llctx());
        case BaseType::TIME:
            return lltype(DataType(types::Converter<ts_t>::btype));
        case BaseType::INDEX:
            return lltype(DataType(types::Converter<idx_t>::btype));
        case BaseType::IVAL:
            return StructType::getTypeByName(llctx(), "struct.ival_t");
        case BaseType::STRUCT: {
            vector<llvm::Type*> lltypes;
            for (auto dt : dtype.dtypes) {
                lltypes.push_back(lltype(dt));
            }
            return StructType::get(llctx(), lltypes);
        }
        case BaseType::PTR:
            return PointerType::get(lltype(dtype.dtypes[0]), 0);
        case BaseType::UNKNOWN:
        default:
            throw std::runtime_error("Invalid type");
    }
}

StructPaddingInfo LLVMTypeGen::getStructPadding(const DataType& dtype)
{
    llvm::LLVMContext llctx;
    unique_ptr<llvm::Module> llmod = make_unique<llvm::Module>("temp", llctx);
    LLVMTypeGen lltypegen(llctx);

    llvm::Type* dtllvm = (llvm::StructType*)lltypegen.lltype(dtype);
    uint64_t total_bytes = llmod->getDataLayout().getTypeAllocSize(dtllvm);

    if(!dtype.is_struct()) {
        return {total_bytes, {}};
    }

    const llvm::StructLayout* sl = llmod->getDataLayout().getStructLayout((llvm::StructType*)dtllvm);

    vector<uint64_t> offsets;
    for (size_t i = 0; i < dtype.dtypes.size(); ++i) {
        offsets.push_back(sl->getElementOffset(i));
    }

    return {total_bytes, offsets};
}
