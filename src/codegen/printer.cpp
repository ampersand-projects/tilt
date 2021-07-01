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
    if (sym.type.dtype.is_ptr) ctx.ostr << "*";
    ctx.ostr << sym.name;
}

void IRPrinter::Visit(const IConst& iconst) { ctx.ostr << iconst.val; }
void IRPrinter::Visit(const UConst& uconst) { ctx.ostr << uconst.val; }
void IRPrinter::Visit(const FConst& fconst) { ctx.ostr << fconst.val; }
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
    emitunary(EXISTS, exists.sym);
}

void IRPrinter::Visit(const Equals& equals)
{
    emitbinary(equals.Left(), "&&", equals.Right());
}

void IRPrinter::Visit(const Not& not_pred)
{
    emitunary("!", not_pred.Input());
}

void IRPrinter::Visit(const And& and_pred)
{
    emitbinary(and_pred.Left(), "&&", and_pred.Right());
}

void IRPrinter::Visit(const Or& or_pred)
{
    emitbinary(or_pred.Left(), "||", or_pred.Right());
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
    emitbinary(lt.Left(), "<", lt.Right());
}

void IRPrinter::Visit(const LessThanEqual& lte)
{
    emitbinary(lte.Left(), "<=", lte.Right());
}

void IRPrinter::Visit(const GreaterThan& gt)
{
    emitbinary(gt.Left(), ">", gt.Right());
}

void IRPrinter::Visit(const AllocIndex& alloc_idx)
{
    emitfunc("alloc_index", { alloc_idx.init_idx });
}

void IRPrinter::Visit(const GetTime& get_time)
{
    emitfunc("get_time", { get_time.idx });
}

void IRPrinter::Visit(const Fetch& fetch)
{
    emitfunc("fetch", { fetch.reg, fetch.idx });
}

void IRPrinter::Visit(const Load& load)
{
    emitfunc("load", { load.ptr });
}

void IRPrinter::Visit(const Advance& adv)
{
    emitfunc("advance", { adv.reg, adv.idx, adv.time });
}

void IRPrinter::Visit(const NextTime& next)
{
    emitfunc("next_time", { next.reg, next.idx });
}

void IRPrinter::Visit(const GetStartIdx& gsi)
{
    emitfunc("get_start_idx", { gsi.reg });
}

void IRPrinter::Visit(const CommitData& commit)
{
    emitfunc("commit", { commit.reg, commit.time, commit.data });
}

void IRPrinter::Visit(const CommitNull& commit)
{
    emitfunc("commit", { commit.reg, commit.time });
}

void IRPrinter::Visit(const AllocRegion& alloc_reg)
{
    emitfunc("alloc_region", { alloc_reg.dur });
}

void IRPrinter::Visit(const MakeRegion& make_reg)
{
    emitfunc("make_region", { make_reg.reg, make_reg.start_idx, make_reg.end_idx });
}

void IRPrinter::Visit(const Call& call)
{
    emitfunc(call.fn->GetName(), call.args);
}

void IRPrinter::Visit(const IfElse& ifelse)
{
    ifelse.cond->Accept(*this);
    ctx.ostr << " ? ";
    ifelse.true_body->Accept(*this);
    ctx.ostr << " : ";
    ifelse.false_body->Accept(*this);
}

void IRPrinter::Visit(const Loop& loop)
{
    for (const auto& inner_loop: loop.inner_loops)
    {
        inner_loop->Accept(*this);
    }

    vector<ExprPtr> args;
    args.insert(args.end(), loop.inputs.begin(), loop.inputs.end());
    emitfunc(loop.GetName(), args);
    emitnewline();

    ctx.ostr << "{";
    enter_block();

    emitcomment("initialization");
    emitnewline();
    for (const auto& [sym, state]: loop.states) {
        emitassign(state.base, state.init);
        emitnewline();
    }
    emitnewline();

    ctx.ostr << "while(1) {";
    enter_block();

    emitcomment("update timer");
    emitnewline();
    emitassign(loop.t, loop.states.at(loop.t).update);
    emitnewline();
    emitnewline();

    emitcomment("loop condition check");
    emitnewline();
    ctx.ostr << "if (";
    loop.exit_cond->Accept(*this);
    ctx.ostr << ") break;";
    emitnewline();
    emitnewline();

    emitcomment("update indices");
    emitnewline();
    for (const auto& idx: loop.idxs) {
        emitassign(idx, loop.states.at(idx).update);
        emitnewline();
    }
    emitnewline();

    emitcomment("set local variables");
    emitnewline();
    for (const auto& [sym, expr]: loop.syms) {
        emitassign(sym, expr);
        emitnewline();
    }
    emitnewline();

    emitcomment("loop body");
    emitnewline();
    emitassign(loop.output, loop.states.at(loop.output).update);
    emitnewline();
    emitnewline();

    emitcomment("Update states");
    for (const auto& [sym, state]: loop.states) {
        emitnewline();
        emitassign(state.base, sym);
    }

    exit_block();
    ctx.ostr << "}";

    exit_block();
    ctx.ostr << "}";
    emitnewline();
    emitnewline();
}