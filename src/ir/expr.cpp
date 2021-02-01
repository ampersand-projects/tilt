#include "till/ir/expr.h"
#include "till/codegen/visitor.h"

using namespace till;

void Symbol::Accept(Visitor& v) const { v.Visit(*this); }
void Lambda::Accept(Visitor& v) const { v.Visit(*this); }
void IConst::Accept(Visitor& v) const { v.Visit(*this); }
void UConst::Accept(Visitor& v) const { v.Visit(*this); }
void FConst::Accept(Visitor& v) const { v.Visit(*this); }
void BConst::Accept(Visitor& v) const { v.Visit(*this); }
void CConst::Accept(Visitor& v) const { v.Visit(*this); }
void Add::Accept(Visitor& v) const { v.Visit(*this); }