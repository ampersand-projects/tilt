#include "till/ir/lstream.h"
#include "till/codegen/visitor.h"

using namespace till;

void SubLStream::Accept(Visitor& v) const { v.Visit(*this); }
void Element::Accept(Visitor& v) const { v.Visit(*this); }