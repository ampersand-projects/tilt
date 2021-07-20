#ifndef INCLUDE_TILT_IR_NODE_H_
#define INCLUDE_TILT_IR_NODE_H_

#include <memory>
#include <map>
#include <vector>
#include <utility>
#include <string>

#include "tilt/base/type.h"

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

    explicit ExprNode(Type type) : type(type) {}

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
    explicit ValNode(DataType dtype) : ExprNode(Type(dtype)) {}
};
typedef shared_ptr<ValNode> Val;

}  // namespace tilt

#endif  // INCLUDE_TILT_IR_NODE_H_
