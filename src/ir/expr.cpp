#include "tilt/ir/expr.h"
#include "tilt/codegen/visitor.h"

using namespace tilt;

void Symbol::Accept(Visitor& v) const { v.Visit(*this); }
void Exists::Accept(Visitor& v) const { v.Visit(*this); }
void Equals::Accept(Visitor& v) const { v.Visit(*this); }
void Not::Accept(Visitor& v) const { v.Visit(*this); }
void And::Accept(Visitor& v) const { v.Visit(*this); }
void Or::Accept(Visitor& v) const { v.Visit(*this); }
void Lambda::Accept(Visitor& v) const { v.Visit(*this); }
void IConst::Accept(Visitor& v) const { v.Visit(*this); }
void UConst::Accept(Visitor& v) const { v.Visit(*this); }
void FConst::Accept(Visitor& v) const { v.Visit(*this); }
void BConst::Accept(Visitor& v) const { v.Visit(*this); }
void CConst::Accept(Visitor& v) const { v.Visit(*this); }
void Add::Accept(Visitor& v) const { v.Visit(*this); }
