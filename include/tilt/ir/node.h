#ifndef TILT_NODE
#define TILT_NODE

#include "tilt/base/type.h"

#include <memory>
#include <map>

using namespace std;

namespace tilt {

    class Visitor;
    struct Expr;
    typedef shared_ptr<Expr> ExprPtr;
    struct Symbol;
    typedef shared_ptr<Symbol> SymPtr;
    typedef vector<SymPtr> Params;
    typedef map<SymPtr, ExprPtr> SymTable;

    struct Expr {
        const Type type;

        Expr(Type type) : type(type) {}

        virtual ~Expr() {}

        SymPtr sym(string name);

        virtual void Accept(Visitor&) const = 0;
    };

    struct Symbol : public Expr {
        const string name;

        Symbol(string name, Type type) :
            Expr(type), name(name)
        {}

        void Accept(Visitor&) const final;
    };

    struct Func : public Expr {
        string name;
        Params inputs;
        SymPtr output;
        SymTable syms;

        Func(string name, Params inputs, SymPtr output, SymTable syms) :
            Expr(output->type), name(name), inputs(move(inputs)), output(output), syms(move(syms))
        {}

        virtual const string GetName() const = 0;

    protected:
        Func(string name, Type type) : Expr(move(type)), name(name) {}
    };
    typedef shared_ptr<Func> FuncPtr;

    struct ValExpr : public Expr {
        ValExpr(DataType dtype) : Expr(Type(dtype)) {}
    };
    typedef shared_ptr<ValExpr> ValExprPtr;

    struct NaryExpr : public ValExpr {
        vector<ExprPtr> inputs;

        NaryExpr(DataType dtype, vector<ExprPtr> inputs)
            : ValExpr(dtype), inputs(move(inputs))
        {}

        template<size_t i>
        ExprPtr Get() const { return inputs[i]; }

        size_t Size() const { return inputs.size(); }
    };

    struct Predicate : public NaryExpr {
        Predicate(vector<ExprPtr> inputs) :
            NaryExpr(types::BOOL, move(inputs))
        {}
    };
    typedef shared_ptr<Predicate> PredPtr;

    struct Const : public ValExpr {
        Const(DataType dtype) : ValExpr(dtype) {}
    };
    typedef shared_ptr<Const> ConstPtr;

} // namespace tilt

#endif // TILT_NODE