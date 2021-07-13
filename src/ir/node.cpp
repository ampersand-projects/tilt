#include "tilt/ir/node.h"
#include "tilt/codegen/visitor.h"

using namespace tilt;

void Symbol::Accept(Visitor& v) const { v.Visit(*this); }

SymPtr Expr::sym(string name) { return make_shared<Symbol>(name, type); }