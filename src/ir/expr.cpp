#include "tilt/ir/expr.h"
#include "tilt/codegen/visitor.h"

using namespace tilt;

void Call::Accept(Visitor& v) const { v.Visit(*this); }
void IfElse::Accept(Visitor& v) const { v.Visit(*this); }
void Exists::Accept(Visitor& v) const { v.Visit(*this); }
void Equals::Accept(Visitor& v) const { v.Visit(*this); }
void Not::Accept(Visitor& v) const { v.Visit(*this); }
void And::Accept(Visitor& v) const { v.Visit(*this); }
void Or::Accept(Visitor& v) const { v.Visit(*this); }
void IConst::Accept(Visitor& v) const { v.Visit(*this); }
void UConst::Accept(Visitor& v) const { v.Visit(*this); }
void FConst::Accept(Visitor& v) const { v.Visit(*this); }
void CConst::Accept(Visitor& v) const { v.Visit(*this); }
void Add::Accept(Visitor& v) const { v.Visit(*this); }
void Sub::Accept(Visitor& v) const { v.Visit(*this); }
void Max::Accept(Visitor& v) const { v.Visit(*this); }
void Min::Accept(Visitor& v) const { v.Visit(*this); }
void True::Accept(Visitor& v) const { v.Visit(*this); }
void False::Accept(Visitor& v) const { v.Visit(*this); }
void LessThan::Accept(Visitor& v) const { v.Visit(*this); }
void LessThanEqual::Accept(Visitor& v) const { v.Visit(*this); }
void GreaterThan::Accept(Visitor& v) const { v.Visit(*this); }
