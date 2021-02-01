#ifndef TILL_IRPRINTER
#define TILL_IRPRINTER

#include "till/codegen/visitor.h"

#include <sstream>
#include <string>

using namespace std;

namespace till
{

    class IRPrinter : public Visitor {
    public:
        IRPrinter() : IRPrinter(0, 2) {}

        void Visit(const Symbol&) override;
        void Visit(const Lambda&) override;
        void Visit(const IConst&) override;
        void Visit(const UConst&) override;
        void Visit(const FConst&) override;
        void Visit(const BConst&) override;
        void Visit(const CConst&) override;
        void Visit(const Add&) override;

        void Visit(const SubLStream&) override;
        void Visit(const Element&) override;

        void Visit(const Op&) override;
        void Visit(const Sum&) override;

        std::string str() const;

    protected:
        IRPrinter(size_t indent, size_t tabstop)
            : indent(indent), tabstop(tabstop)
        {}

        void emitnewline();
        void emittab();
        void enter_block();
        void exit_block();

    private:
        size_t indent;
        size_t tabstop;
        ostringstream ostr;
    };

} // namespace till

#endif // TILL_IRPRINTER