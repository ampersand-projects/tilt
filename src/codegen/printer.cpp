#include "tilt/codegen/printer.h"

using namespace tilt;
using namespace std;

static const string EXISTS = u8"\u2203";
static const string FORALL = u8"\u2200";
static const string IN = u8"\u2208";
static const string PHI = u8"\u0278";

string idx_str(long idx)
{
    ostringstream ostr;
    if (idx > 0) { ostr << " + " << idx; }
    else if (idx < 0) { ostr << " - " << -idx; }
    return ostr.str();
}

std::string IRPrinter::str() const { return ostr.str(); }
void IRPrinter::emittab() { ostr << string(1 << tabstop, ' '); }
void IRPrinter::emitnewline() { ostr << endl << string(indent << tabstop, ' '); }
void IRPrinter::enter_block() { indent++; emitnewline(); }
void IRPrinter::exit_block() { indent--; emitnewline(); }

void IRPrinter::Visit(const Symbol& sym)
{
    if (sym.type.tl.iters.size()) ostr << "~";
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
void IRPrinter::Visit(const Now&)
{
    ostr << "t" << nesting;
}

void IRPrinter::Visit(const Exists& exists)
{
    ostr << EXISTS;
    exists.expr->Accept(*this);
}

void IRPrinter::Visit(const Equals& equals)
{
    ostr << "(";
    equals.a->Accept(*this);
    ostr << " == ";
    equals.b->Accept(*this);
    ostr << ")";
}

void IRPrinter::Visit(const Not& not_pred)
{
    ostr << "!";
    not_pred.a->Accept(*this);
}

void IRPrinter::Visit(const And& and_pred)
{
    ostr << "(";
    and_pred.a->Accept(*this);
    ostr << " && ";
    and_pred.b->Accept(*this);
    ostr << ")";
}

void IRPrinter::Visit(const Or& or_pred)
{
    ostr << "(";
    or_pred.a->Accept(*this);
    ostr << " || ";
    or_pred.b->Accept(*this);
    ostr << ")";
}

void IRPrinter::Visit(const Lambda& lambda)
{
    lambda.pred->Accept(*this);
    ostr << " ? ";
    lambda.output->Accept(*this);
    ostr << " : " << PHI;
}

void IRPrinter::Visit(const SubLStream& subls)
{
    subls.lstream->Accept(*this);
    ostr << "[t" << nesting << idx_str(subls.win.start.offset)
        << " : t" << nesting << idx_str(subls.win.end.offset) << "]";
}

void IRPrinter::Visit(const Element& elem)
{
    elem.lstream->Accept(*this);
    ostr << "[t" << nesting;
    ostr << idx_str(elem.pt.offset);
    ostr << "]";
}

void IRPrinter::Visit(const Op& op)
{
    nesting++;
    auto off = op.iter.offset;
    auto per = op.iter.period;
    ostr << FORALL << "t" << nesting << " " << IN << " ~(" << off << "," << per << ") ";

    ostr << "[";
    for (auto in : op.inputs) {
        in->Accept(*this);
        ostr << ";";
    }
    ostr << "] ";

    ostr << "[";
    for (auto it : op.type.tl.iters) {
        ostr << " ~(" << it->offset << "," << it->period << ");";
    }
    ostr << "] {";

    enter_block();
    for (auto& it : op.syms) {
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
    nesting--;
}

void IRPrinter::Visit(const Sum& sum)
{
    ostr << "+{";
    enter_block();
    sum.op->Accept(*this);
    exit_block();
    ostr << "}";
    emitnewline();
}