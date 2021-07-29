#include <unordered_set>

#include "tilt/codegen/printer.h"
#include "tilt/builder/tilder.h"

using namespace tilt;
using namespace std;

static const auto EXISTS = "\u2203";
static const auto FORALL = "\u2200";
static const auto IN = "\u2208";
static const auto PHI = "\u0278";

string idx_str(int64_t idx)
{
    ostringstream ostr;
    if (idx > 0) {
        ostr << " + " << idx;
    } else if (idx < 0) {
        ostr << " - " << -idx;
    }
    return ostr.str();
}

void IRPrinter::Visit(const Symbol& sym)
{
    if (!sym.type.is_valtype()) ostr << "~";
    if (sym.type.dtype.is_ptr()) ostr << "*";
    ostr << sym.name;
}

void IRPrinter::Visit(const Exists& exists)
{
    emitunary(EXISTS, exists.sym);
}

void IRPrinter::Visit(const ConstNode& cnst)
{
    switch (cnst.type.dtype.btype) {
        case BaseType::BOOL: ostr << (cnst.val ? "true" : "false"); break;
        case BaseType::INT8:
        case BaseType::INT16:
        case BaseType::INT32:
        case BaseType::INT64: ostr << cnst.val << "i"; break;
        case BaseType::UINT8:
        case BaseType::UINT16:
        case BaseType::UINT32:
        case BaseType::UINT64: ostr << cnst.val << "u"; break;
        case BaseType::FLOAT32:
        case BaseType::FLOAT64: ostr << cnst.val << "f"; break;
        case BaseType::TIME: ostr << cnst.val << "t"; break;
        default: throw std::runtime_error("Invalid constant type"); break;
    }
}

void IRPrinter::Visit(const NaryExpr& e)
{
    switch (e.op) {
        case MathOp::ADD: emitbinary(e.arg(0), "+", e.arg(1)); break;
        case MathOp::SUB: emitbinary(e.arg(0), "-", e.arg(1)); break;
        case MathOp::MUL: emitbinary(e.arg(0), "*", e.arg(1)); break;
        case MathOp::DIV: emitbinary(e.arg(0), "/", e.arg(1)); break;
        case MathOp::MOD: emitbinary(e.arg(0), "%", e.arg(1)); break;
        case MathOp::MAX: emitfunc("max", {e.arg(0), e.arg(1)}); break;
        case MathOp::MIN: emitfunc("min", {e.arg(0), e.arg(1)}); break;
        case MathOp::SQRT: emitfunc("sqrt", {e.arg(0)}); break;
        case MathOp::POW: emitfunc("pow", {e.arg(0), e.arg(1)}); break;
        case MathOp::EQ: emitbinary(e.arg(0), "==", e.arg(1)); break;
        case MathOp::NOT: emitunary("!", e.arg(0)); break;
        case MathOp::AND: emitbinary(e.arg(0), "&&", e.arg(1)); break;
        case MathOp::OR: emitbinary(e.arg(0), "||", e.arg(1)); break;
        case MathOp::LT: emitbinary(e.arg(0), "<", e.arg(1)); break;
        case MathOp::LTE: emitbinary(e.arg(0), "<=", e.arg(1)); break;
        case MathOp::GT: emitbinary(e.arg(0), ">", e.arg(1)); break;
        case MathOp::GTE: emitbinary(e.arg(0), ">=", e.arg(1)); break;
        default: throw std::runtime_error("Invalid math operation"); break;
    }
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

void IRPrinter::Visit(const OpNode& op)
{
    enter_op();
    ostr << FORALL << "t" << ctx.nesting << " " << IN << " " << op.iter.name << " ";

    ostr << "[";
    for (auto in : op.inputs) {
        in->Accept(*this);
        ostr << "; ";
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

void IRPrinter::Visit(const AggNode& agg)
{
    ostr << "[init: s = ";
    agg.init->Accept(*this);
    ostr << "] ";

    ostr << "[acc: s = ";
    auto state_sym = agg.init->sym("s");
    auto output_sym = agg.op->output->sym("o");
    auto acc_expr = agg.acc(state_sym, output_sym);
    acc_expr->Accept(*this);
    ostr << "] {";

    enter_block();
    agg.op->Accept(*this);
    exit_block();
    ostr << "}";
}

void IRPrinter::Visit(const AllocIndex& alloc_idx)
{
    emitfunc("alloc_index", { alloc_idx.init_idx });
}

void IRPrinter::Visit(const GetTime& get_time)
{
    emitfunc("get_time", { get_time.idx });
}

void IRPrinter::Visit(const GetIndex& get_index)
{
    emitfunc("get_index", { get_index.idx });
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
    emitfunc("alloc_region", { alloc_reg.size });
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

void IRPrinter::Visit(const Get& get)
{
    emitfunc("get", {get.input, tilder::_u64(get.n)});
}

void IRPrinter::Visit(const New& _new)
{
    emitfunc("new", _new.inputs);
}

void IRPrinter::Visit(const Loop& loop)
{
    for (const auto& inner_loop : loop.inner_loops)
    {
        inner_loop->Accept(*this);
    }

    vector<Expr> args;
    args.insert(args.end(), loop.inputs.begin(), loop.inputs.end());
    emitfunc(loop.GetName(), args);
    emitnewline();

    ostr << "{";
    enter_block();

    emitcomment("initialization");
    emitnewline();
    unordered_set<Sym> bases;
    for (const auto& [_, base] : loop.state_bases) {
        emitassign(base, loop.syms.at(base));
        bases.insert(base);
        emitnewline();
    }
    emitnewline();

    ostr << "while(1) {";
    enter_block();

    emitcomment("loop condition check");
    emitnewline();
    ostr << "if (";
    loop.exit_cond->Accept(*this);
    ostr << ") break;";
    emitnewline();
    emitnewline();

    emitcomment("update timer");
    emitnewline();
    emitassign(loop.t, loop.syms.at(loop.t));
    emitnewline();
    emitnewline();

    emitcomment("update indices");
    emitnewline();
    for (const auto& idx : loop.idxs) {
        emitassign(idx, loop.syms.at(idx));
        emitnewline();
    }
    emitnewline();

    emitcomment("set local variables");
    emitnewline();
    for (const auto& [sym, expr] : loop.syms) {
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
    for (const auto& [var, base] : loop.state_bases) {
        emitnewline();
        emitassign(base, var);
    }

    exit_block();
    ostr << "}";

    emitnewline();
    emitnewline();
    ostr << "return ";
    loop.state_bases.at(loop.output)->Accept(*this);
    ostr << ";";

    exit_block();
    ostr << "}";
    emitnewline();
    emitnewline();
}

string IRPrinter::Build(const Expr expr)
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
