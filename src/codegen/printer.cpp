#include "tilt/codegen/printer.h"

using namespace tilt;
using namespace std;

string idx_str(long idx)
{
    ostringstream ostr;
    if (idx > 0) { ostr << "+" << idx; }
    else if (idx < 0) { ostr << idx; }
    return ostr.str();
}

std::string IRPrinter::str() const { return ostr.str(); }
void IRPrinter::emittab() { ostr << string(1 << tabstop, ' '); }
void IRPrinter::emitnewline() { ostr << endl << string(indent << tabstop, ' '); }
void IRPrinter::enter_block() { indent++; emitnewline(); }
void IRPrinter::exit_block() { indent--; emitnewline(); }

void IRPrinter::Visit(const Symbol& sym)
{
    if (sym.type.iter.period) ostr << "~";
    ostr << sym.name;
}

void IRPrinter::Visit(const IConst& iconst) { ostr << iconst.val; }
void IRPrinter::Visit(const UConst& uconst) { ostr << uconst.val; }
void IRPrinter::Visit(const FConst& fconst) { ostr << fconst.val; }
void IRPrinter::Visit(const BConst& bconst) { ostr << bconst.val; }
void IRPrinter::Visit(const CConst& cconst) { ostr << cconst.val; }
void IRPrinter::Visit(const Add& add)
{
    ostr << "(";
    add.Left()->Accept(*this);
    ostr << "+";
    add.Right()->Accept(*this);
    ostr << ")";
}

void IRPrinter::Visit(const Lambda& lambda)
{
    ostr << "(";
    lambda.output->Accept(*this);
    ostr << ")";
}

void IRPrinter::Visit(const SubLStream& subls)
{
    subls.lstream->Accept(*this);
    ostr << "[t" << indent << idx_str(subls.win.shift)
        << ":t" << indent << idx_str(subls.win.len) << "]";
}

void IRPrinter::Visit(const Element& elem)
{
    elem.lstream->Accept(*this);
    ostr << "[t" << indent;
    ostr << idx_str(elem.pt.shift);
    ostr << "]";
}

void IRPrinter::Visit(const Op& op)
{
    auto off = op.type.iter.offset;
    auto per = op.type.iter.period;
    ostr << "forall t" << indent << " in ~(" << off << "," << per << ") ";

    ostr << "[";
    for (auto in : op.inputs) {
        in->Accept(*this);
        ostr << ";";
    }
    ostr << "] {";
    enter_block();
    for (auto& it : op.vars) {
        it.first->Accept(*this);
        ostr << " = ";
        it.second->Accept(*this);
        emitnewline();
    }
    ostr << "return ";
    op.output->Accept(*this);
    exit_block();
    ostr << "}";
    emitnewline();
}

void IRPrinter::Visit(const Sum& sum)
{
    ostr << "+ {";
    enter_block();
    sum.op->Accept(*this);
    exit_block();
    ostr << "}";
    emitnewline();
}