#include "tilt/codegen/printer.h"

#include <unordered_set>

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
    if (sym.type.dtype.is_ptr) ostr << "*";
    ostr << sym.name;
}

void IRPrinter::Visit(const IConst& iconst) { ostr << iconst.val; }
void IRPrinter::Visit(const UConst& uconst) { ostr << uconst.val; }
void IRPrinter::Visit(const FConst& fconst) { ostr << fconst.val; }
void IRPrinter::Visit(const CConst& cconst) { ostr << cconst.val; }
void IRPrinter::Visit(const TConst& tconst) { ostr << tconst.val; }
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
    ostr << "t" << ctx.nesting;
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
    ostr << "[init: s = ";
    agg.init->Accept(*this);
    ostr << "] ";

    ostr << "[acc: s = ";
    auto state_sym = agg.init->GetSym("s");
    auto output_sym = agg.op->output->GetSym("o");
    auto acc_expr = agg.acc(state_sym, output_sym);
    acc_expr->Accept(*this);
    ostr << "] {";

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

void IRPrinter::Visit(const Store& store)
{
    emitfunc("store", { store.reg, store.ptr, store.data });
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

void IRPrinter::Visit(const GetEndIdx& gei)
{
    emitfunc("get_end_idx", { gei.reg });
}

void IRPrinter::Visit(const CommitData& commit)
{
    emitfunc("commit_data", { commit.reg, commit.time });
}

void IRPrinter::Visit(const CommitNull& commit)
{
    emitfunc("commit_null", { commit.reg, commit.time });
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
    ostr << " ? ";
    ifelse.true_body->Accept(*this);
    ostr << " : ";
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

    ostr << "{";
    enter_block();

    emitcomment("initialization");
    emitnewline();
    unordered_set<SymPtr> bases;
    for (const auto& [_, base]: loop.state_bases) {
        emitassign(base, loop.syms.at(base));
        bases.insert(base);
        emitnewline();
    }
    emitnewline();

    ostr << "while(1) {";
    enter_block();

    emitcomment("update timer");
    emitnewline();
    emitassign(loop.t, loop.syms.at(loop.t));
    emitnewline();
    emitnewline();

    emitcomment("loop condition check");
    emitnewline();
    ostr << "if (";
    loop.exit_cond->Accept(*this);
    ostr << ") break;";
    emitnewline();
    emitnewline();

    emitcomment("update indices");
    emitnewline();
    for (const auto& idx: loop.idxs) {
        emitassign(idx, loop.syms.at(idx));
        emitnewline();
    }
    emitnewline();

    emitcomment("set local variables");
    emitnewline();
    for (const auto& [sym, expr]: loop.syms) {
        if (bases.find(sym) == bases.end() &&
            loop.state_bases.find(sym) == loop.state_bases.end()) {
            emitassign(sym, expr);
            emitnewline();
        }
    }
    emitnewline();

    emitcomment("loop body");
    emitnewline();
    emitassign(loop.output, loop.syms.at(loop.output));
    emitnewline();
    emitnewline();

    emitcomment("Update states");
    for (const auto& [var, base]: loop.state_bases) {
        emitnewline();
        emitassign(base, var);
    }

    exit_block();
    ostr << "}";

    exit_block();
    ostr << "}";
    emitnewline();
    emitnewline();
}

string IRPrinter::Build(const ExprPtr expr)
{
    IRPrinter printer;
    expr->Accept(printer);
    return printer.ostr.str();
}

string IRPrinter::Build(const llvm::Module* mod)
{
    std::string str;
    llvm::raw_string_ostream ostr(str);
    ostr << *mod;
    ostr.flush();
    return ostr.str();
}