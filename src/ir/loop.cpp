#include "tilt/ir/loop.h"
#include "tilt/codegen/visitor.h"

using namespace tilt;

void GetTime::Accept(Visitor& v) const { v.Visit(*this); }
void Fetch::Accept(Visitor& v) const { v.Visit(*this); }
void Advance::Accept(Visitor& v) const { v.Visit(*this); }
void Next::Accept(Visitor& v) const { v.Visit(*this); }
void CommitData::Accept(Visitor& v) const { v.Visit(*this); }
void CommitNull::Accept(Visitor& v) const { v.Visit(*this); }
void Block::Accept(Visitor& v) const { v.Visit(*this); }
void Loop::Accept(Visitor& v) const { v.Visit(*this); }
void TConst::Accept(Visitor& v) const { v.Visit(*this); }