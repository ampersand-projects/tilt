#include "tilt/ir/expr.h"
#include "tilt/ir/lstream.h"
#include "tilt/ir/op.h"
#include "tilt/ir/loop.h"
#include "tilt/codegen/visitor.h"

using namespace tilt;

void Symbol::Accept(Visitor& v) const { v.Visit(*this); }
void Call::Accept(Visitor& v) const { v.Visit(*this); }
void IfElse::Accept(Visitor& v) const { v.Visit(*this); }
void Get::Accept(Visitor& v) const { v.Visit(*this); }
void New::Accept(Visitor& v) const { v.Visit(*this); }
void Exists::Accept(Visitor& v) const { v.Visit(*this); }
void ConstNode::Accept(Visitor& v) const { v.Visit(*this); }
void NaryExpr::Accept(Visitor& v) const { v.Visit(*this); }
void SubLStream::Accept(Visitor& v) const { v.Visit(*this); }
void Element::Accept(Visitor& v) const { v.Visit(*this); }
void OpNode::Accept(Visitor& v) const { v.Visit(*this); }
void AggNode::Accept(Visitor& v) const { v.Visit(*this); }
void AllocIndex::Accept(Visitor& v) const { v.Visit(*this); }
void GetTime::Accept(Visitor& v) const { v.Visit(*this); }
void GetIndex::Accept(Visitor& v) const { v.Visit(*this); }
void Fetch::Accept(Visitor& v) const { v.Visit(*this); }
void Load::Accept(Visitor& v) const { v.Visit(*this); }
void Store::Accept(Visitor& v) const { v.Visit(*this); }
void Advance::Accept(Visitor& v) const { v.Visit(*this); }
void NextTime::Accept(Visitor& v) const { v.Visit(*this); }
void GetStartIdx::Accept(Visitor& v) const { v.Visit(*this); }
void GetEndIdx::Accept(Visitor& v) const { v.Visit(*this); }
void CommitData::Accept(Visitor& v) const { v.Visit(*this); }
void CommitNull::Accept(Visitor& v) const { v.Visit(*this); }
void AllocRegion::Accept(Visitor& v) const { v.Visit(*this); }
void MakeRegion::Accept(Visitor& v) const { v.Visit(*this); }
void Loop::Accept(Visitor& v) const { v.Visit(*this); }
