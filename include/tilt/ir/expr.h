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

    struct Call : public Expr {
        FuncPtr fn;
        vector<ExprPtr> args;

        Call(FuncPtr fn, vector<ExprPtr> args) :
            Expr(fn->type), fn(fn), args(move(args))
        {}

        void Accept(Visitor&) const final;
    };
    typedef shared_ptr<Call> CallPtr;

    struct IfElse : public Expr {
        ExprPtr cond;
        ExprPtr true_body;
        ExprPtr false_body;

        IfElse(ExprPtr cond, ExprPtr true_body, ExprPtr false_body) :
            Expr(true_body->type), cond(cond), true_body(true_body), false_body(false_body)
        {
            assert(cond->type == types::BOOL);
            assert(true_body->type.dtype == false_body->type.dtype);
        }

        void Accept(Visitor&) const final;
    };

    struct Now : public ValExpr {
        Now() : ValExpr(types::TIMESTAMP) {}

        void Accept(Visitor&) const final;
    };
    typedef shared_ptr<Now> NowPtr;

    struct UnaryPred : public Predicate {
        UnaryPred(ExprPtr a) : Predicate({a}) {}

        ExprPtr Input() const { return Get<0>(); }
    };

    struct BinaryPred : public Predicate {
        BinaryPred(ExprPtr a, ExprPtr b) : Predicate({a, b}) {}

        ExprPtr Left() const { return Get<0>(); }
        ExprPtr Right() const { return Get<1>(); }
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
        SymPtr sym;

        Exists(SymPtr sym) : UnaryPred({(ExprPtr) sym}), sym(sym) {}

        void Accept(Visitor&) const final;
    };

    struct Not : public UnaryPred {
        Not(ExprPtr a) : UnaryPred(a) {}

        void Accept(Visitor&) const final;
    };

    struct Equals : public BinaryPred {
        Equals(ExprPtr a, ExprPtr b) : BinaryPred(a, b) {}

        void Accept(Visitor&) const final;
    };

    struct And : public BinaryPred {
        And(ExprPtr a, ExprPtr b) : BinaryPred(a, b) {}

        void Accept(Visitor&) const final;
    };

    struct Or : public BinaryPred {
        Or(ExprPtr a, ExprPtr b) : BinaryPred(a, b) {}

        void Accept(Visitor&) const final;
    };

    struct LessThan : public BinaryPred {
        LessThan(ExprPtr a, ExprPtr b) : BinaryPred(a, b) {}

        void Accept(Visitor&) const final;
    };

    struct GreaterThan : public BinaryPred {
        GreaterThan(ExprPtr a, ExprPtr b) : BinaryPred(a, b) {}

        void Accept(Visitor&) const final;
    };

    struct LessThanEqual : public BinaryPred {
        LessThanEqual(ExprPtr a, ExprPtr b) : BinaryPred(a, b) {}

        void Accept(Visitor&) const final;
    };

    struct IConst : public Const {
        const int64_t val;

        IConst(DataType dtype, const int64_t val) :
            Const(dtype), val(val)
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

    struct UConst : public Const {
        const uint64_t val;

        UConst(DataType dtype, const uint64_t val) :
            Const(dtype), val(val)
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

    struct FConst : public Const {
        const double val;

        FConst(DataType dtype, const double val) :
            Const(dtype), val(val)
        {
            assert(
                dtype == types::FLOAT32 ||
                dtype == types::FLOAT64
            );
        }

        void Accept(Visitor&) const final;
    };

    struct CConst : public Const {
        const char val;

        CConst(const char val) :
            Const(types::CHAR), val(val)
        {}

        void Accept(Visitor&) const final;
    };

    struct TConst : public Const {
        const long val;

        TConst(const long val) :
            Const(types::TIME), val(val)
        {}

        void Accept(Visitor&) const final;
    };

    struct UnaryExpr : public NaryExpr {
        UnaryExpr(DataType dtype, ExprPtr input)
            : NaryExpr(dtype, vector<ExprPtr>{input})
        {}

        ExprPtr Input() const { return Get<0>(); }
    };

    struct BinaryExpr : public NaryExpr {
        BinaryExpr(DataType dtype, ExprPtr left, ExprPtr right)
            : NaryExpr(dtype, vector<ExprPtr>{left, right})
        {}

        ExprPtr Left() const { return Get<0>(); }
        ExprPtr Right() const { return Get<1>(); }
    };

    struct Add : public BinaryExpr {
        Add(ExprPtr a, ExprPtr b) :
            BinaryExpr(a->type.dtype, a, b)
        {
            assert(a->type.dtype == b->type.dtype);
        }

        void Accept(Visitor&) const final;
    };

    struct Sub : public BinaryExpr {
        Sub(ExprPtr a, ExprPtr b) :
            BinaryExpr(a->type.dtype, a, b)
        {
            assert(a->type.dtype == b->type.dtype);
        }

        void Accept(Visitor&) const final;
    };

    struct Max : public BinaryExpr {
        Max(ExprPtr a, ExprPtr b) :
            BinaryExpr(a->type.dtype, a, b)
        {
            assert(a->type.dtype == b->type.dtype);
        }

        void Accept(Visitor&) const final;
    };

    struct Min : public BinaryExpr {
        Min(ExprPtr a, ExprPtr b) :
            BinaryExpr(a->type.dtype, a, b)
        {
            assert(a->type.dtype == b->type.dtype);
        }

        void Accept(Visitor&) const final;
    };

} // namespace tilt

#endif // TILT_EXPR
