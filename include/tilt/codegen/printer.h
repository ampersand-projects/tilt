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

        friend class IRPrinter;
    };

    class IRPrinter : public Visitor {
    public:
        IRPrinter(IRPrinterCtx ctx) : IRPrinter(move(ctx), 2) {}

        IRPrinter(IRPrinterCtx ctx, size_t tabstop) :
            ctx(move(ctx)), tabstop(tabstop)
        {}

        string result() const { return ostr.str(); }

        /**
         * TiLT IR
         */
        void Visit(const Symbol&) override;
        void Visit(const Lambda&) override;
        void Visit(const Exists&) override;
        void Visit(const Equals&) override;
        void Visit(const Not&) override;
        void Visit(const And&) override;
        void Visit(const Or&) override;
        void Visit(const IConst&) override;
        void Visit(const UConst&) override;
        void Visit(const FConst&) override;
        void Visit(const BConst&) override;
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

        void Visit(const SubLStream&) override;
        void Visit(const Element&) override;

        void Visit(const Op&) override;
        void Visit(const AggExpr&) override;

        /**
         * Loop IR
         */
        void Visit(const GetTime&) override;
        void Visit(const Fetch&) override;
        void Visit(const Advance&) override;
        void Visit(const Next&) override;
        void Visit(const CommitData&) override;
        void Visit(const CommitNull&) override;
        void Visit(const Block&) override;
        void Visit(const Loop&) override;

    private:
        void enter_op() { ctx.nesting++; }
        void exit_op() { ctx.nesting--; }
        void enter_block() { ctx.indent++; emitnewline(); }
        void exit_block() { ctx.indent--; emitnewline(); }

        void emittab() { ostr << string(1 << tabstop, ' '); }
        void emitnewline() { ostr << endl << string(ctx.indent << tabstop, ' '); }
        void emitcomment(string comment) { ostr << "/* " << comment << " */"; }

        void emitunary(string op, ASTPtr a)
        {
            ostr << op;
            a->Accept(*this);
        }

        void emitbinary(ASTPtr a, string op, ASTPtr b)
        {
            a->Accept(*this);
            ostr << " " << op << " ";
            b->Accept(*this);
        }

        void emitassign(ASTPtr lhs, ASTPtr rhs)
        {
            lhs->Accept(*this);
            ostr << " = ";
            rhs->Accept(*this);
            ostr << ";";
        }

        void emitfunc(string name, vector<ASTPtr> args)
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
        ostringstream ostr;
        size_t tabstop;
    };

} // namespace tilt

#endif // TILT_IRPRINTER