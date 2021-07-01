#include "tilt/ir/loop.h"
#include "tilt/codegen/visitor.h"

using namespace tilt;

void AllocIndex::Accept(Visitor& v) const { v.Visit(*this); }
void GetTime::Accept(Visitor& v) const { v.Visit(*this); }
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
void TConst::Accept(Visitor& v) const { v.Visit(*this); }