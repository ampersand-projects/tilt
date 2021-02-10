#include "tilt/codegen/llvmgen.h"

using namespace tilt;
using namespace std;

void LLVMGen::Visit(const Symbol& sym)
{
}

void LLVMGen::Visit(const IConst& iconst) {  }
void LLVMGen::Visit(const UConst& uconst) {  }
void LLVMGen::Visit(const FConst& fconst) {  }
void LLVMGen::Visit(const BConst& bconst) {  }
void LLVMGen::Visit(const CConst& cconst) {  }
void LLVMGen::Visit(const Add& add)
{

}

void LLVMGen::Visit(const Exists& exists)
{
}

void LLVMGen::Visit(const Equals& equals)
{
}

void LLVMGen::Visit(const Not& not_pred)
{
}

void LLVMGen::Visit(const And& and_pred)
{
}

void LLVMGen::Visit(const Or& or_pred)
{
}

void LLVMGen::Visit(const Lambda& lambda)
{
}

void LLVMGen::Visit(const SubLStream& subls)
{
}

void LLVMGen::Visit(const Element& elem)
{
}

void LLVMGen::Visit(const Op& op)
{

}

void LLVMGen::Visit(const Sum& sum)
{

}