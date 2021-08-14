#ifndef INCLUDE_TILT_CODEGEN_PRINTER_H_
#define INCLUDE_TILT_CODEGEN_PRINTER_H_

#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "llvm/IR/Module.h"
#include "tilt/codegen/visitor.h"

using namespace std;

namespace tilt {

class IRPrinterCtx {
public:
    IRPrinterCtx() : indent(0), nesting(0) {}

private:
    size_t indent;
    size_t nesting;

    friend class IRPrinter;
};

class IRPrinter : public Visitor {
public:
    IRPrinter() : IRPrinter(IRPrinterCtx()) {}
    explicit IRPrinter(IRPrinterCtx ctx) : IRPrinter(move(ctx), 2) {}

    IRPrinter(IRPrinterCtx ctx, size_t tabstop) :
        ctx(move(ctx)), tabstop(tabstop)
    {}

    static string Build(const Expr);
    static string Build(const llvm::Module*);

    void Visit(const Symbol&) override;
    void Visit(const Out&) override;
    void Visit(const Beat&) override;
    void Visit(const Call&) override;
    void Visit(const IfElse&) override;
    void Visit(const Select&) override;
    void Visit(const Get&) override;
    void Visit(const New&) override;
    void Visit(const Exists&) override;
    void Visit(const ConstNode&) override;
    void Visit(const Cast&) override;
    void Visit(const NaryExpr&) override;
    void Visit(const SubLStream&) override;
    void Visit(const Element&) override;
    void Visit(const OpNode&) override;
    void Visit(const Reduce&) override;
    void Visit(const Fetch&) override;
    void Visit(const Read&) override;
    void Visit(const Write&) override;
    void Visit(const Advance&) override;
    void Visit(const GetCkpt&) override;
    void Visit(const GetStartIdx&) override;
    void Visit(const GetEndIdx&) override;
    void Visit(const GetStartTime&) override;
    void Visit(const GetEndTime&) override;
    void Visit(const CommitData&) override;
    void Visit(const CommitNull&) override;
    void Visit(const AllocRegion&) override;
    void Visit(const MakeRegion&) override;
    void Visit(const LoopNode&) override;

private:
    void enter_op() { ctx.nesting++; }
    void exit_op() { ctx.nesting--; }
    void enter_block() { ctx.indent++; emitnewline(); }
    void exit_block() { ctx.indent--; emitnewline(); }

    void emittab() { ostr << string(1 << tabstop, ' '); }
    void emitnewline() { ostr << endl << string(ctx.indent << tabstop, ' '); }
    void emit(string str) { ostr << str; }
    void emitcomment(string comment) { ostr << "/* " << comment << " */"; }

    void emitunary(const string op, const Expr a)
    {
        ostr << op;
        a->Accept(*this);
    }

    void emitbinary(const Expr a, const string op, const Expr b)
    {
        ostr << "(";
        a->Accept(*this);
        ostr << " " << op << " ";
        b->Accept(*this);
        ostr << ")";
    }

    void emitassign(const Expr lhs, const Expr rhs)
    {
        lhs->Accept(*this);
        ostr << " = ";
        rhs->Accept(*this);
        ostr << ";";
    }

    void emitfunc(const string name, const vector<Expr> args)
    {
        ostr << name << "(";
        for (size_t i = 0; i < args.size()-1; i++) {
            args[i]->Accept(*this);
            ostr << ", ";
        }
        args.back()->Accept(*this);
        ostr << ")";
    }

    IRPrinterCtx ctx;
    size_t tabstop;
    ostringstream ostr;
};

}  // namespace tilt

#endif  // INCLUDE_TILT_CODEGEN_PRINTER_H_
