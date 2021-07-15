#ifndef TILT_NODE
#define TILT_NODE

#include "tilt/base/type.h"

#include <memory>
#include <map>

using namespace std;

namespace tilt {

    class Visitor;
    struct ExprNode;
    typedef shared_ptr<ExprNode> Expr;
    struct Symbol;
    typedef shared_ptr<Symbol> Sym;
    typedef vector<Sym> Params;
    typedef map<Sym, Expr> SymTable;

    struct ExprNode {
        const Type type;

        ExprNode(Type type) : type(type) {}

        virtual ~ExprNode() {}

        Sym sym(string name) { return make_shared<Symbol>(name, type); }

        virtual void Accept(Visitor&) const = 0;
    };

    struct Symbol : public ExprNode {
        const string name;

        Symbol(string name, Type type) : ExprNode(type), name(name) {}

        void Accept(Visitor&) const final;
    };

    struct FuncNode : public ExprNode {
        string name;
        Params inputs;
        Sym output;
        SymTable syms;

        FuncNode(string name, Params inputs, Sym output, SymTable syms) :
            ExprNode(output->type), name(name), inputs(move(inputs)), output(output), syms(move(syms))
        {}

        virtual const string GetName() const = 0;

    protected:
        FuncNode(string name, Type type) : ExprNode(move(type)), name(name) {}
    };
    typedef shared_ptr<FuncNode> Func;

    struct ValNode : public ExprNode {
        ValNode(DataType dtype) : ExprNode(Type(dtype)) {}
    };
    typedef shared_ptr<ValNode> Val;

} // namespace tilt

#endif // TILT_NODE