#include "tilt/ir/lstream.h"
#include "tilt/codegen/visitor.h"

using namespace tilt;

void SubLStream::Accept(Visitor& v) const { v.Visit(*this); }
void Element::Accept(Visitor& v) const { v.Visit(*this); }