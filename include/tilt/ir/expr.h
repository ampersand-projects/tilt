#ifndef INCLUDE_TILT_IR_EXPR_H_
#define INCLUDE_TILT_IR_EXPR_H_

#include <string>
#include <memory>
#include <vector>
#include <map>
#include <utility>

#include "tilt/base/type.h"
#include "tilt/base/log.h"
#include "tilt/ir/node.h"

using namespace std;

namespace tilt {

struct Call : public ExprNode {
    string name;
    vector<Expr> args;

    Call(string name, Type type, vector<Expr> args) :
        ExprNode(type), name(name), args(move(args))
    {}

    void Accept(Visitor&) const final;
};

struct Select : public ValNode {
    Expr cond;
    Expr true_body;
    Expr false_body;

    Select(Expr cond, Expr true_body, Expr false_body) :
        ValNode(true_body->type.dtype), cond(cond), true_body(true_body), false_body(false_body)
    {
        ASSERT(cond->type.dtype == types::BOOL);
        ASSERT(true_body->type.dtype == false_body->type.dtype);
    }

    void Accept(Visitor&) const final;
};

struct Read : public ValNode {
    Expr ptr;

    explicit Read(Expr ptr) : ValNode(ptr->type.dtype.deref()), ptr(ptr) {}

    void Accept(Visitor&) const final;
};

struct Get : public ValNode {
    Expr input;
    size_t n;

    Get(Expr input, size_t n) :
        ValNode(input->type.dtype.dtypes[n]), input(input), n(n)
    {
        ASSERT(input->type.dtype.is_struct());
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
    Expr expr;

    explicit Exists(Expr expr) : ValNode(types::BOOL), expr(expr) {}

    void Accept(Visitor&) const final;
};

struct Cast : public ValNode {
    Expr arg;

    Cast(DataType dtype, Expr arg) : ValNode(dtype), arg(arg) {}

    void Accept(Visitor&) const final;
};

struct NaryExpr : public ValNode {
    MathOp op;
    vector<Expr> args;

    NaryExpr(DataType dtype, MathOp op, vector<Expr> args) :
        ValNode(dtype), op(op), args(move(args))
    {
        ASSERT(!arg(0)->type.dtype.is_ptr() && !arg(0)->type.dtype.is_struct());
    }

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
        ASSERT(left->type == right->type);
    }
};

struct Not : public UnaryExpr {
    explicit Not(Expr a) : UnaryExpr(types::BOOL, MathOp::NOT, a)
    {
        ASSERT(a->type.dtype == types::BOOL);
    }
};

struct Neg : public UnaryExpr {
    explicit Neg(Expr a) : UnaryExpr(a->type.dtype, MathOp::NEG, a) {}
};

struct Sqrt : public UnaryExpr {
    explicit Sqrt(Expr a) : UnaryExpr(a->type.dtype, MathOp::SQRT, a) {}
};

struct Ceil : public UnaryExpr {
    explicit Ceil(Expr a) : UnaryExpr(a->type.dtype, MathOp::CEIL, a) {
        ASSERT(a->type.dtype.is_float());
    }
};

struct Floor : public UnaryExpr {
    explicit Floor(Expr a) : UnaryExpr(a->type.dtype, MathOp::FLOOR, a) {
        ASSERT(a->type.dtype.is_float());
    }
};

struct Equals : public BinaryExpr {
    Equals(Expr a, Expr b) : BinaryExpr(types::BOOL, MathOp::EQ, a, b) {}
};

struct And : public BinaryExpr {
    And(Expr a, Expr b) : BinaryExpr(types::BOOL, MathOp::AND, a, b)
    {
        ASSERT(a->type.dtype == types::BOOL);
    }
};

struct Or : public BinaryExpr {
    Or(Expr a, Expr b) : BinaryExpr(types::BOOL, MathOp::OR, a, b)
    {
        ASSERT(a->type.dtype == types::BOOL);
    }
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

struct Mod : public BinaryExpr {
    Mod(Expr a, Expr b) : BinaryExpr(a->type.dtype, MathOp::MOD, a, b)
    {
        ASSERT(!a->type.dtype.is_float());
    }
};

struct Pow : public BinaryExpr {
    Pow(Expr a, Expr b) : BinaryExpr(a->type.dtype, MathOp::POW, a, b) {
        ASSERT(a->type.dtype.is_float());
    }
};

}  // namespace tilt

#endif  // INCLUDE_TILT_IR_EXPR_H_
