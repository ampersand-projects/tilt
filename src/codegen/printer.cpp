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
    if (sym.type.tl.iters.size()) ctx.ostr << "~";
    ctx.ostr << sym.name;
}

void IRPrinter::Visit(const IConst& iconst) { ctx.ostr << iconst.val; }
void IRPrinter::Visit(const UConst& uconst) { ctx.ostr << uconst.val; }
void IRPrinter::Visit(const FConst& fconst) { ctx.ostr << fconst.val; }
void IRPrinter::Visit(const BConst& bconst) { ctx.ostr << bconst.val; }
void IRPrinter::Visit(const CConst& cconst) { ctx.ostr << cconst.val; }
void IRPrinter::Visit(const TConst& tconst) { ctx.ostr << tconst.val; }
void IRPrinter::Visit(const Add& add)
{
    emitbinary(add.Left(), "+", add.Right());
}
void IRPrinter::Visit(const Sub& sub)
{
    emitbinary(sub.Left(), "-", sub.Right());
}
void IRPrinter::Visit(const Max& max)
{
    emitfunc("max", {max.Left(), max.Right()});
}
void IRPrinter::Visit(const Min& min)
{
    emitfunc("min", {min.Left(), min.Right()});
}
void IRPrinter::Visit(const Now&)
{
    ctx.ostr << "t" << ctx.nesting;
}

void IRPrinter::Visit(const Exists& exists)
{
    emitunary(EXISTS, exists.expr);
}

void IRPrinter::Visit(const Equals& equals)
{
    emitbinary(equals.a, "&&", equals.b);
}

void IRPrinter::Visit(const Not& not_pred)
{
    emitunary("!", not_pred.a);
}

void IRPrinter::Visit(const And& and_pred)
{
    emitbinary(and_pred.a, "&&", and_pred.b);
}

void IRPrinter::Visit(const Or& or_pred)
{
    emitbinary(or_pred.a, "||", or_pred.b);
}

void IRPrinter::Visit(const Lambda& lambda)
{
    lambda.output->Accept(*this);
}

void IRPrinter::Visit(const SubLStream& subls)
{
    subls.lstream->Accept(*this);
    ctx.ostr << "[t" << ctx.nesting << idx_str(subls.win.start.offset)
        << " : t" << ctx.nesting << idx_str(subls.win.end.offset) << "]";
}

void IRPrinter::Visit(const Element& elem)
{
    elem.lstream->Accept(*this);
    ctx.ostr << "[t" << ctx.nesting;
    ctx.ostr << idx_str(elem.pt.offset);
    ctx.ostr << "]";
}

void IRPrinter::Visit(const Op& op)
{
    enter_op();
    ctx.ostr << FORALL << "t" << ctx.nesting << " " << IN << " " << op.iter.name << " ";

    ctx.ostr << "[";
    for (auto in : op.inputs) {
        in->Accept(*this);
        ctx.ostr << "; ";
    }
    ctx.ostr << "] ";

    ctx.ostr << "[";
    for (auto it : op.type.tl.iters) {
        ctx.ostr << it.name << "; ";
    }
    ctx.ostr << "] {";

    enter_block();
    for (auto& it : op.syms) {
        it.first->Accept(*this);
        ctx.ostr << " = ";
        it.second->Accept(*this);
        emitnewline();
    }
    ctx.ostr << "return ";
    op.pred->Accept(*this);
    ctx.ostr << " ? ";
    op.output->Accept(*this);
    ctx.ostr << " : " << PHI;
    exit_block();
    
    ctx.ostr << "}";
    exit_op();
}

void IRPrinter::Visit(const AggExpr& agg)
{
    switch (agg.agg)
    {
    case AggType::SUM:
        ctx.ostr << "+";
        break;
    default:
        ctx.ostr << "unk";
        break;
    }
    ctx.ostr << "{";
    enter_block();
    agg.op->Accept(*this);
    exit_block();
    ctx.ostr << "}";
}

void IRPrinter::Visit(const True&)
{
    ctx.ostr << "True";
}

void IRPrinter::Visit(const False&)
{
    ctx.ostr << "False";
}

void IRPrinter::Visit(const LessThan& lt)
{
    emitbinary(lt.a, "<", lt.b);
}

void IRPrinter::Visit(const LessThanEqual& lte)
{
    emitbinary(lte.a, "<=", lte.b);
}

void IRPrinter::Visit(const GetTime& get_time)
{
    emitfunc("get_time", {get_time.idx});
}

void IRPrinter::Visit(const Fetch& fetch)
{
    emitfunc("fetch", {fetch.reg, fetch.idx});
}

void IRPrinter::Visit(const Advance& adv)
{
    emitfunc("advance", {adv.reg, adv.idx, adv.time});
}

void IRPrinter::Visit(const Next& next)
{
    emitfunc("next", {next.reg, next.idx});
}

void IRPrinter::Visit(const CommitData& commit)
{
    emitfunc("commit", {commit.reg, commit.time, commit.data});
}

void IRPrinter::Visit(const CommitNull& commit)
{
    emitfunc("commit", {commit.reg, commit.time});
}

void IRPrinter::Visit(const Block& block)
{
    ctx.ostr << "{";
    enter_block();
    for (auto& stmt : block.stmts) {
        stmt->Accept(*this);
        emitnewline();
    }
    exit_block();
    ctx.ostr << "}";
    emitnewline();
}

void IRPrinter::Visit(const Loop& loop)
{
    vector<ASTPtr> args = {loop.t_start, loop.t_end, loop.out_reg};
    args.insert(args.end(), loop.in_regs.begin(), loop.in_regs.end());
    emitfunc("void loop_" + loop.name, args);
    emitnewline();

    ctx.ostr << "{";
    enter_block();

    emitcomment("initialization");
    emitnewline();
    emitassign(loop.t_cur, loop.t_start);
    emitnewline();

    for (const auto& [idx, reg]: loop.idx_map) {
        idx->Accept(*this);
        ctx.ostr << " = ";
        reg->Accept(*this);
        ctx.ostr << "->start_idx;";
        emitnewline();
    }
    emitnewline();

    ctx.ostr << "While(1) {";
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
    ctx.ostr << "if (";
    emitbinary(loop.t_cur, ">", loop.t_end);
    ctx.ostr << ") break;";
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
    ctx.ostr << "if (";
    loop.pred->Accept(*this);
    ctx.ostr << ") {";
    enter_block();
    loop.true_body->Accept(*this);
    exit_block();
    ctx.ostr << "} else {";
    enter_block();
    loop.false_body->Accept(*this);
    exit_block();
    ctx.ostr << "}";

    exit_block();
    ctx.ostr << "}";

    exit_block();
    ctx.ostr << "}";
    emitnewline();
}