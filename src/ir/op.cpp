#include "tilt/ir/op.h"
#include "tilt/codegen/visitor.h"

using namespace tilt;

void Op::Accept(Visitor& v) const { v.Visit(*this); }
void AggExpr::Accept(Visitor& v) const { v.Visit(*this); }