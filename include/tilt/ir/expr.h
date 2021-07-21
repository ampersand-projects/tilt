#ifndef INCLUDE_TILT_IR_EXPR_H_
#define INCLUDE_TILT_IR_EXPR_H_

#include <string>
#include <memory>
#include <cassert>
#include <vector>
#include <map>
#include <utility>

#include "tilt/base/type.h"
#include "tilt/ir/node.h"

using namespace std;

namespace tilt {

struct Call : public ExprNode {
    Func fn;
    vector<Expr> args;

    Call(Func fn, vector<Expr> args) :
        ExprNode(fn->type), fn(fn), args(move(args))
    {}

    void Accept(Visitor&) const final;
};

struct IfElse : public ExprNode {
    Expr cond;
    Expr true_body;
    Expr false_body;

    IfElse(Expr cond, Expr true_body, Expr false_body) :
        ExprNode(true_body->type), cond(cond), true_body(true_body), false_body(false_body)
    {
        assert(cond->type.dtype == types::BOOL);
        assert(true_body->type.dtype == false_body->type.dtype);
    }

    void Accept(Visitor&) const final;
};

struct Get : public ValNode {
    Expr input;
    size_t n;

    Get(Expr input, size_t n) :
        ValNode(input->type.dtype.dtypes[n]), input(input), n(n)
    {
        assert(input->type.dtype.is_struct());
    }

    void Accept(Visitor&) const final;
};

struct New : public ValNode {
    vector<Expr> inputs;

    explicit New(vector<Expr> inputs) :
        ValNode(get_new_type(inputs)), inputs(inputs)
    {}

    void Accept(Visitor&) const final;

private:
    static DataType get_new_type(vector<Expr> inputs)
    {
        vector<DataType> dtypes;
        for (const auto& input : inputs) {
            dtypes.push_back(input->type.dtype);
        }
        return DataType(BaseType::STRUCT, (dtypes));
    }
};

struct ConstNode : public ValNode {
    const double val;

    ConstNode(BaseType btype, double val) :
        ValNode(DataType(btype)), val(val)
    {}

    void Accept(Visitor&) const final;
};
typedef shared_ptr<ConstNode> Const;

struct Exists : public ValNode {
    Sym sym;

    explicit Exists(Sym sym) : ValNode(types::BOOL), sym(sym) {}

    void Accept(Visitor&) const final;
};

struct NaryExpr : public ValNode {
    MathOp op;
    vector<Expr> args;

    NaryExpr(DataType dtype, MathOp op, vector<Expr> args) :
        ValNode(dtype), op(op), args(move(args))
    {}

    Expr arg(size_t i) const { return args[i]; }

    size_t size() const { return args.size(); }

    void Accept(Visitor&) const final;
};

struct UnaryExpr : public NaryExpr {
    UnaryExpr(DataType dtype, MathOp op, Expr input)
        : NaryExpr(dtype, op, vector<Expr>{input})
    {}
};

struct BinaryExpr : public NaryExpr {
    BinaryExpr(DataType dtype, MathOp op, Expr left, Expr right)
        : NaryExpr(dtype, op, vector<Expr>{left, right})
    {
        assert(left->type == right->type);
    }
};

struct Not : public UnaryExpr {
    explicit Not(Expr a) : UnaryExpr(types::BOOL, MathOp::NOT, a) {}
};

struct Equals : public BinaryExpr {
    Equals(Expr a, Expr b) : BinaryExpr(types::BOOL, MathOp::EQ, a, b) {}
};

struct And : public BinaryExpr {
    And(Expr a, Expr b) : BinaryExpr(types::BOOL, MathOp::AND, a, b) {}
};

struct Or : public BinaryExpr {
    Or(Expr a, Expr b) : BinaryExpr(types::BOOL, MathOp::OR, a, b) {}
};

struct LessThan : public BinaryExpr {
    LessThan(Expr a, Expr b) : BinaryExpr(types::BOOL, MathOp::LT, a, b) {}
};

struct GreaterThan : public BinaryExpr {
    GreaterThan(Expr a, Expr b) : BinaryExpr(types::BOOL, MathOp::GT, a, b) {}
};

struct LessThanEqual : public BinaryExpr {
    LessThanEqual(Expr a, Expr b) : BinaryExpr(types::BOOL, MathOp::LTE, a, b) {}
};

struct GreaterThanEqual : public BinaryExpr {
    GreaterThanEqual(Expr a, Expr b) : BinaryExpr(types::BOOL, MathOp::GTE, a, b) {}
};

struct Add : public BinaryExpr {
    Add(Expr a, Expr b) : BinaryExpr(a->type.dtype, MathOp::ADD, a, b) {}
};

struct Sub : public BinaryExpr {
    Sub(Expr a, Expr b) : BinaryExpr(a->type.dtype, MathOp::SUB, a, b) {}
};

struct Mul : public BinaryExpr {
    Mul(Expr a, Expr b) : BinaryExpr(a->type.dtype, MathOp::MUL, a, b) {}
};

struct Div : public BinaryExpr {
    Div(Expr a, Expr b) : BinaryExpr(a->type.dtype, MathOp::DIV, a, b) {}
};

struct Max : public BinaryExpr {
    Max(Expr a, Expr b) : BinaryExpr(a->type.dtype, MathOp::MAX, a, b) {}
};

struct Min : public BinaryExpr {
    Min(Expr a, Expr b) : BinaryExpr(a->type.dtype, MathOp::MIN, a, b) {}
};

}  // namespace tilt

#endif  // INCLUDE_TILT_IR_EXPR_H_
