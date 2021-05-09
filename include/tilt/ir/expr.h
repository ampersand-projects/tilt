#ifndef TILT_EXPR
#define TILT_EXPR

#include "tilt/base/type.h"
#include "tilt/codegen/node.h"

#include <string>
#include <memory>
#include <cassert>
#include <vector>
#include <map>

using namespace std;

namespace tilt {

    struct Symbol;
    
    struct Expr : public ASTNode {
        const Type type;

        Expr(Type type) : type(type) {}

        shared_ptr<Symbol> GetSym(string name)
        {
            return make_shared<Symbol>(name, type);
        }
    };
    typedef shared_ptr<Expr> ExprPtr;

    struct Symbol : public Expr {
        const string name;

        Symbol(string name, Type type) :
            Expr(type), name(name)
        {}

        void Accept(Visitor&) const final;
    };
    typedef shared_ptr<Symbol> SymPtr;

    typedef vector<SymPtr> Params; // Input parameters to the Op
    typedef map<SymPtr, ExprPtr> SymTable;

    struct ValExpr : public Expr {
        ValExpr(DataType dtype) : Expr(move(Type(dtype))) {}
    };
    typedef shared_ptr<ValExpr> ValExprPtr;

    struct Now : public ValExpr {
        Now() : ValExpr(types::TIMESTAMP) {}

        void Accept(Visitor&) const final;
    };
    typedef shared_ptr<Now> NowPtr;

    struct Predicate : public ValExpr {
        Predicate() : ValExpr(types::BOOL) {}
    };
    typedef shared_ptr<Predicate> PredPtr;

    struct True : public Predicate {
        void Accept(Visitor&) const final;
    };
    struct False : public Predicate {
        void Accept(Visitor&) const final;
    };

    struct Exists : public Predicate {
        ExprPtr expr;

        Exists(ExprPtr expr) : expr(expr) {}

        void Accept(Visitor&) const final;
    };

    struct Equals : public Predicate {
        ExprPtr a;
        ExprPtr b;

        Equals(ExprPtr a, ExprPtr b) : a(a), b(b) {}

        void Accept(Visitor&) const final;
    };

    struct UnaryPred : public Predicate {
        PredPtr a;

        UnaryPred(PredPtr a) : a(a) {}
    };

    struct Not : public UnaryPred {
        Not(PredPtr a) : UnaryPred(a) {}

        void Accept(Visitor&) const final;
    };

    struct BinaryPred : public Predicate {
        PredPtr a;
        PredPtr b;

        BinaryPred(PredPtr a, PredPtr b) : a(a), b(b) {}
    };

    struct And : public BinaryPred {
        And(PredPtr a, PredPtr b) : BinaryPred(a, b) {}

        void Accept(Visitor&) const final;
    };

    struct Or : public BinaryPred {
        Or(PredPtr a, PredPtr b) : BinaryPred(a, b) {}

        void Accept(Visitor&) const final;
    };

    struct Compare : public Predicate {
        ValExprPtr a;
        ValExprPtr b;

        Compare(ValExprPtr a, ValExprPtr b) : a(a), b(b) {}
    };

    struct LessThan : public Compare {
        LessThan(ValExprPtr a, ValExprPtr b) : Compare(a, b) {}

        void Accept(Visitor&) const final;
    };

    struct LessThanEqual : public Compare {
        LessThanEqual(ValExprPtr a, ValExprPtr b) : Compare(a, b) {}

        void Accept(Visitor&) const final;
    };

    struct Lambda : public ValExpr {
        Params inputs;
        ExprPtr output;

        Lambda(Params inputs, ExprPtr output) :
            ValExpr(output->type.dtype), inputs(move(inputs)), output(output)
        {}

        void Accept(Visitor&) const final;
    };
    typedef shared_ptr<Lambda> LambdaExpr;

    struct Const : public ValExpr {
        Const(DataType dtype) : ValExpr(dtype) {}
    };
    typedef shared_ptr<Const> ConstPtr;

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

    struct BConst : public Const {
        const bool val;

        BConst(const bool val) :
            Const(types::BOOL), val(val)
        {}

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

    struct NaryExpr : public ValExpr {
        vector<ExprPtr> inputs;

        NaryExpr(DataType dtype, vector<ExprPtr> inputs)
            : ValExpr(dtype), inputs(move(inputs))
        {}

        template<size_t n>
        ExprPtr Get() const { return inputs[n]; }
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