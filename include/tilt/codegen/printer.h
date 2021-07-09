#ifndef TILT_IRPRINTER
#define TILT_IRPRINTER

#include "tilt/codegen/visitor.h"

#include <sstream>
#include <string>

using namespace std;

namespace tilt
{

    class IRPrinterCtx {
    public:
        IRPrinterCtx() : indent(0), nesting(0) {}

    private:
        size_t indent;
        size_t nesting;
        ostringstream ostr;

        friend class IRPrinter;
    };

    class IRPrinter : public Visitor {
    public:
        IRPrinter() : IRPrinter(IRPrinterCtx()) {}
        IRPrinter(IRPrinterCtx ctx) : IRPrinter(move(ctx), 2) {}

        IRPrinter(IRPrinterCtx ctx, size_t tabstop) :
            ctx(move(ctx)), tabstop(tabstop)
        {}

        static string Build(const ExprPtr);
        static string Build(const llvm::Module*);

        void Visit(const Symbol&) override;
        void Visit(const Call&) override;
        void Visit(const IfElse&) override;
        void Visit(const Exists&) override;
        void Visit(const Equals&) override;
        void Visit(const Not&) override;
        void Visit(const And&) override;
        void Visit(const Or&) override;
        void Visit(const IConst&) override;
        void Visit(const UConst&) override;
        void Visit(const FConst&) override;
        void Visit(const CConst&) override;
        void Visit(const TConst&) override;
        void Visit(const Add&) override;
        void Visit(const Sub&) override;
        void Visit(const Max&) override;
        void Visit(const Min&) override;
        void Visit(const Now&) override;
        void Visit(const True&) override;
        void Visit(const False&) override;
        void Visit(const LessThan&) override;
        void Visit(const LessThanEqual&) override;
        void Visit(const GreaterThan&) override;
        void Visit(const SubLStream&) override;
        void Visit(const Element&) override;
        void Visit(const Op&) override;
        void Visit(const AggExpr&) override;
        void Visit(const AllocIndex&) override;
        void Visit(const GetTime&) override;
        void Visit(const Fetch&) override;
        void Visit(const Load&) override;
        void Visit(const Store&) override;
        void Visit(const Advance&) override;
        void Visit(const NextTime&) override;
        void Visit(const GetStartIdx&) override;
        void Visit(const GetEndIdx&) override;
        void Visit(const CommitData&) override;
        void Visit(const CommitNull&) override;
        void Visit(const AllocRegion&) override;
        void Visit(const MakeRegion&) override;
        void Visit(const Loop&) override;

    private:
        void enter_op() { ctx.nesting++; }
        void exit_op() { ctx.nesting--; }
        void enter_block() { ctx.indent++; emitnewline(); }
        void exit_block() { ctx.indent--; emitnewline(); }

        void emittab() { ctx.ostr << string(1 << tabstop, ' '); }
        void emitnewline() { ctx.ostr << endl << string(ctx.indent << tabstop, ' '); }
        void emit(string str) { ctx.ostr << str; }
        void emitcomment(string comment) { ctx.ostr << "/* " << comment << " */"; }

        void emitunary(const string op, const ExprPtr a)
        {
            ctx.ostr << op;
            a->Accept(*this);
        }

        void emitbinary(const ExprPtr a, const string op, const ExprPtr b)
        {
            a->Accept(*this);
            ctx.ostr << " " << op << " ";
            b->Accept(*this);
        }

        void emitassign(const ExprPtr lhs, const ExprPtr rhs)
        {
            lhs->Accept(*this);
            ctx.ostr << " = ";
            rhs->Accept(*this);
            ctx.ostr << ";";
        }

        void emitfunc(const string name, const vector<ExprPtr> args)
        {
            ctx.ostr << name << "(";
            for (size_t i = 0; i < args.size()-1; i++) {
                args[i]->Accept(*this);
                ctx.ostr << ", ";
            }
            args.back()->Accept(*this);
            ctx.ostr << ")";
        }

        IRPrinterCtx ctx;
        size_t tabstop;
    };

} // namespace tilt

#endif // TILT_IRPRINTER
