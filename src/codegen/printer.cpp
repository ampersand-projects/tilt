#include "tilt/codegen/printer.h"

using namespace tilt;
using namespace std;

static const auto EXISTS = "\u2203";
static const auto FORALL = "\u2200";
static const auto IN = "\u2208";
static const auto PHI = "\u0278";

string idx_str(long idx)
{
    ostringstream ostr;
    if (idx > 0) { ostr << " + " << idx; }
    else if (idx < 0) { ostr << " - " << -idx; }
    return ostr.str();
}

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
void IRPrinter::Visit(const TConst& tconst) { ostr << tconst.val; }
void IRPrinter::Visit(const Add& add)
{
    ostr << "(";
    add.Left()->Accept(*this);
    ostr << " + ";
    add.Right()->Accept(*this);
    ostr << ")";
}
void IRPrinter::Visit(const Sub& add)
{
    ostr << "(";
    add.Left()->Accept(*this);
    ostr << " - ";
    add.Right()->Accept(*this);
    ostr << ")";
}
void IRPrinter::Visit(const Max& max)
{
    ostr << "max(";
    max.Left()->Accept(*this);
    ostr << ", ";
    max.Right()->Accept(*this);
    ostr << ")";
}
void IRPrinter::Visit(const Min& max)
{
    ostr << "min(";
    max.Left()->Accept(*this);
    ostr << ", ";
    max.Right()->Accept(*this);
    ostr << ")";
}
void IRPrinter::Visit(const Now&)
{
    ostr << "t" << ctx.nesting;
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
    lambda.output->Accept(*this);
}

void IRPrinter::Visit(const SubLStream& subls)
{
    subls.lstream->Accept(*this);
    ostr << "[t" << ctx.nesting << idx_str(subls.win.start.offset)
        << " : t" << ctx.nesting << idx_str(subls.win.end.offset) << "]";
}

void IRPrinter::Visit(const Element& elem)
{
    elem.lstream->Accept(*this);
    ostr << "[t" << ctx.nesting;
    ostr << idx_str(elem.pt.offset);
    ostr << "]";
}

void IRPrinter::Visit(const Op& op)
{
    enter_op();
    ostr << FORALL << "t" << ctx.nesting << " " << IN << " " << op.iter.name << " ";

    ostr << "[";
    for (auto in : op.inputs) {
        in->Accept(*this);
        ostr << "; ";
    }
    ostr << "] ";

    ostr << "[";
    for (auto it : op.type.tl.iters) {
        ostr << it.name << "; ";
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
    op.pred->Accept(*this);
    ostr << " ? ";
    op.output->Accept(*this);
    ostr << " : " << PHI;
    exit_block();
    
    ostr << "}";
    exit_op();
}

void IRPrinter::Visit(const AggExpr& agg)
{
    switch (agg.agg)
    {
    case AggType::SUM:
        ostr << "+";
        break;
    default:
        ostr << "unk";
        break;
    }
    ostr << "{";
    enter_block();
    agg.op->Accept(*this);
    exit_block();
    ostr << "}";
}

void IRPrinter::Visit(const True&)
{
    ostr << "True";
}

void IRPrinter::Visit(const False&)
{
    ostr << "False";
}

void IRPrinter::Visit(const LessThan& lt)
{
    ostr << "(";
    lt.a->Accept(*this);
    ostr << " < ";
    lt.b->Accept(*this);
    ostr << ")";
}

void IRPrinter::Visit(const LessThanEqual& lte)
{
    ostr << "(";
    lte.a->Accept(*this);
    ostr << " <= ";
    lte.b->Accept(*this);
    ostr << ")";
}

void IRPrinter::Visit(const GetTime& get_time)
{
    ostr << "get_time" << "(";
    get_time.idx->Accept(*this);
    ostr << ")";
}

void IRPrinter::Visit(const Fetch& fetch)
{
    ostr << "fetch" << "(";
    fetch.reg->Accept(*this);
    ostr << ", ";
    fetch.idx->Accept(*this);
    ostr << ")";
}

void IRPrinter::Visit(const Advance& adv)
{
    ostr << "advance" << "(";
    adv.reg->Accept(*this);
    ostr << ", ";
    adv.idx->Accept(*this);
    ostr << ", ";
    adv.time->Accept(*this);
    ostr << ")";
}

void IRPrinter::Visit(const Next& next)
{
    ostr << "next" << "(";
    next.idx->Accept(*this);
    ostr << ")";
}

void IRPrinter::Visit(const CommitData& commit)
{
    ostr << "commit_data" << "(";
    commit.region->Accept(*this);
    ostr << ", ";
    commit.time->Accept(*this);
    ostr << ", ";
    commit.data->Accept(*this);
    ostr << ")";
}

void IRPrinter::Visit(const CommitNull& commit)
{
    ostr << "commit_null" << "(";
    commit.region->Accept(*this);
    ostr << ", ";
    commit.time->Accept(*this);
    ostr << ")";
}

void IRPrinter::Visit(const Block& block)
{
    ostr << "{";
    enter_block();
    for (auto& stmt : block.stmts) {
        stmt->Accept(*this);
        emitnewline();
    }
    exit_block();
    ostr << "}";
    emitnewline();
}

void IRPrinter::Visit(const Loop& loop)
{
    vector<ASTPtr> args = {loop.t_start, loop.t_end, loop.out_reg};
    args.insert(args.end(), loop.in_regs.begin(), loop.in_regs.end());
    emitfunc("void loop_" + loop.name, args);
    emitnewline();

    ostr << "{";
    enter_block();

    emitcomment("initialization");
    emitnewline();
    emitassign(loop.t_cur, loop.t_start);
    emitnewline();

    for (const auto& [idx, reg]: loop.idx_map) {
        idx->Accept(*this);
        ostr << " = ";
        reg->Accept(*this);
        ostr << "->start_idx;";
        emitnewline();
    }
    emitnewline();

    ostr << "While(1) {";
    enter_block();

    emitcomment("update timers");
    emitnewline();
    emitassign(loop.t_prev, loop.t_cur);
    emitnewline();

    emitassign(loop.t_cur, loop.next_t);
    emitnewline();
    emitnewline();

    emitcomment("loop condition check");
    emitnewline();
    ostr << "if (";
    emitbinary(loop.t_cur, ">", loop.t_end);
    ostr << ") break;";
    emitnewline();
    emitnewline();

    emitcomment("update indices");
    emitnewline();
    for (const auto& [idx, expr]: loop.idx_update) {
        emitassign(idx, expr);
        emitnewline();
    }
    emitnewline();

    emitcomment("set local variables");
    emitnewline();
    for (const auto& [sym, expr]: loop.vars) {
        emitassign(sym, expr);
        emitnewline();
    }
    emitnewline();

    emitcomment("loop body");
    emitnewline();
    ostr << "if (";
    loop.pred->Accept(*this);
    ostr << ") {";
    enter_block();
    loop.true_body->Accept(*this);
    exit_block();
    ostr << "} else {";
    enter_block();
    loop.false_body->Accept(*this);
    exit_block();
    ostr << "}";

    exit_block();
    ostr << "}";

    exit_block();
    ostr << "}";
    emitnewline();
}