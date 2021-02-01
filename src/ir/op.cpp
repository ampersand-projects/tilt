#include "till/ir/op.h"
#include "till/codegen/visitor.h"

using namespace till;

void Op::Accept(Visitor& v) const { v.Visit(*this); }
void Sum::Accept(Visitor& v) const { v.Visit(*this); }
