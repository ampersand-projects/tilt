#ifndef TILT_EXPR
#define TILT_EXPR

#include "tilt/base/type.h"
#include "tilt/ir/node.h"

#include <string>
#include <memory>
#include <cassert>
#include <vector>
#include <map>

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
            assert(cond->type == types::BOOL);
            assert(true_body->type.dtype == false_body->type.dtype);
        }

        void Accept(Visitor&) const final;
    };

    struct Get : public ValNode {
        Expr input;
        size_t n;

        Get(Expr input, size_t n) :
            ValNode(input->type.dtype.ptypes[n]), input(input), n(n)
        {
            assert(n < input->type.dtype.ptypes.size());
        }

        void Accept(Visitor&) const final;
    };

    struct New : public ValNode {
        vector<Expr> inputs;

        New(vector<Expr> inputs) :
            ValNode(get_new_type(inputs)), inputs(inputs)
        {}

        void Accept(Visitor&) const final;

    private:
        static DataType get_new_type(vector<Expr> inputs)
        {
            vector<PrimitiveType> ptypes;
            for (const auto& input: inputs) {
                auto pts = input->type.dtype.ptypes;
                assert(pts.size() == 1);
                ptypes.push_back(pts[0]);
            }
            return DataType(move(ptypes));
        }
    };

    struct ConstNode : public ValNode {
        ConstNode(DataType dtype) : ValNode(dtype) {}
    };
    typedef shared_ptr<ConstNode> Const;

    struct IConst : public ConstNode {
        const int64_t val;

        IConst(DataType dtype, const int64_t val) :
            ConstNode(dtype), val(val)
        {
            assert(
                dtype == types::INT8 ||
                dtype == types::INT16 ||
                dtype == types::INT32 ||
                dtype == types::INT64
            );
        }

        void Accept(Visitor&) const final;
    };

    struct UConst : public ConstNode {
        const uint64_t val;

        UConst(DataType dtype, const uint64_t val) :
            ConstNode(dtype), val(val)
        {
            assert(
                dtype == types::UINT8 ||
                dtype == types::UINT16 ||
                dtype == types::UINT32 ||
                dtype == types::UINT64
            );
        }

        void Accept(Visitor&) const final;
    };

    struct FConst : public ConstNode {
        const double val;

        FConst(DataType dtype, const double val) :
            ConstNode(dtype), val(val)
        {
            assert(
                dtype == types::FLOAT32 ||
                dtype == types::FLOAT64
            );
        }

        void Accept(Visitor&) const final;
    };

    struct CConst : public ConstNode {
        const char val;

        CConst(const char val) :
            ConstNode(types::CHAR), val(val)
        {}

        void Accept(Visitor&) const final;
    };

    struct TConst : public ConstNode {
        const long val;

        TConst(const long val) :
            ConstNode(types::TIME), val(val)
        {}

        void Accept(Visitor&) const final;
    };

    struct NaryExpr : public ValNode {
        vector<Expr> inputs;

        NaryExpr(DataType dtype, vector<Expr> inputs)
            : ValNode(dtype), inputs(move(inputs))
        {}

        template<size_t i>
        Expr Get() const { return inputs[i]; }

        size_t Size() const { return inputs.size(); }
    };

    struct Predicate : public NaryExpr {
        Predicate(vector<Expr> inputs) :
            NaryExpr(types::BOOL, move(inputs))
        {}
    };
    typedef shared_ptr<Predicate> Pred;

    struct UnaryPred : public Predicate {
        UnaryPred(Expr a) : Predicate({a}) {}

        Expr Input() const { return Get<0>(); }
    };

    struct BinaryPred : public Predicate {
        BinaryPred(Expr a, Expr b) : Predicate({a, b}) {}

        Expr Left() const { return Get<0>(); }
        Expr Right() const { return Get<1>(); }
    };

    struct True : public Predicate {
        True() : Predicate({}) {}

        void Accept(Visitor&) const final;
    };

    struct False : public Predicate {
        False() : Predicate({}) {}

        void Accept(Visitor&) const final;
    };

    struct Exists : public UnaryPred {
        Sym sym;

        Exists(Sym sym) : UnaryPred({(Expr) sym}), sym(sym) {}

        void Accept(Visitor&) const final;
    };

    struct Not : public UnaryPred {
        Not(Expr a) : UnaryPred(a) {}

        void Accept(Visitor&) const final;
    };

    struct Equals : public BinaryPred {
        Equals(Expr a, Expr b) : BinaryPred(a, b) {}

        void Accept(Visitor&) const final;
    };

    struct And : public BinaryPred {
        And(Expr a, Expr b) : BinaryPred(a, b) {}

        void Accept(Visitor&) const final;
    };

    struct Or : public BinaryPred {
        Or(Expr a, Expr b) : BinaryPred(a, b) {}

        void Accept(Visitor&) const final;
    };

    struct LessThan : public BinaryPred {
        LessThan(Expr a, Expr b) : BinaryPred(a, b) {}

        void Accept(Visitor&) const final;
    };

    struct GreaterThan : public BinaryPred {
        GreaterThan(Expr a, Expr b) : BinaryPred(a, b) {}

        void Accept(Visitor&) const final;
    };

    struct LessThanEqual : public BinaryPred {
        LessThanEqual(Expr a, Expr b) : BinaryPred(a, b) {}

        void Accept(Visitor&) const final;
    };

    struct UnaryExpr : public NaryExpr {
        UnaryExpr(DataType dtype, Expr input)
            : NaryExpr(dtype, vector<Expr>{input})
        {}

        Expr Input() const { return Get<0>(); }
    };

    struct BinaryExpr : public NaryExpr {
        BinaryExpr(DataType dtype, Expr left, Expr right)
            : NaryExpr(dtype, vector<Expr>{left, right})
        {}

        Expr Left() const { return Get<0>(); }
        Expr Right() const { return Get<1>(); }
    };

    struct Add : public BinaryExpr {
        Add(Expr a, Expr b) :
            BinaryExpr(a->type.dtype, a, b)
        {
            assert(a->type.dtype == b->type.dtype);
        }

        void Accept(Visitor&) const final;
    };

    struct Sub : public BinaryExpr {
        Sub(Expr a, Expr b) :
            BinaryExpr(a->type.dtype, a, b)
        {
            assert(a->type.dtype == b->type.dtype);
        }

        void Accept(Visitor&) const final;
    };

    struct Max : public BinaryExpr {
        Max(Expr a, Expr b) :
            BinaryExpr(a->type.dtype, a, b)
        {
            assert(a->type.dtype == b->type.dtype);
        }

        void Accept(Visitor&) const final;
    };

    struct Min : public BinaryExpr {
        Min(Expr a, Expr b) :
            BinaryExpr(a->type.dtype, a, b)
        {
            assert(a->type.dtype == b->type.dtype);
        }

        void Accept(Visitor&) const final;
    };

    struct Now : public ValNode {
        Now() : ValNode(types::TIMESTAMP) {}

        void Accept(Visitor&) const final;
    };

} // namespace tilt

#endif // TILT_EXPR
