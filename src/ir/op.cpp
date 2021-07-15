#include "tilt/ir/op.h"
#include "tilt/codegen/visitor.h"

using namespace tilt;

void OpNode::Accept(Visitor& v) const { v.Visit(*this); }
void AggNode::Accept(Visitor& v) const { v.Visit(*this); }
