#ifndef TILT_LOOP
#define TILT_LOOP

#include "tilt/ir/node.h"
#include "tilt/ir/expr.h"

#include <map>
#include <memory>
#include <vector>
#include <cassert>

using namespace std;

namespace tilt
{

    struct Time : public Symbol {
        Time(string name) : Symbol(name, types::TIME) {}
    };
    typedef shared_ptr<Time> Timer;

    struct Index : public Symbol {
        Index(string name) : Symbol(name, types::INDEX_PTR) {}
    };
    typedef shared_ptr<Index> Indexer;

    struct Region : public Symbol {
        Region(string name, Type type) :
            Symbol(name, move(type))
        {
            assert(type.isLStream());
        }
    };
    typedef shared_ptr<Region> RegPtr;

    struct AllocIndex : public ValExpr {
        ExprPtr init_idx;

        AllocIndex(ExprPtr init_idx) :
            ValExpr(types::INDEX_PTR), init_idx(init_idx)
        {
            assert(init_idx->type == types::INDEX_PTR);
        }

        void Accept(Visitor&) const final;
    };

    struct GetTime : public ValExpr {
        ExprPtr idx;

        GetTime(ExprPtr idx) :
            ValExpr(types::TIME), idx(idx)
        {
            assert(idx->type == types::INDEX_PTR);
        }

        void Accept(Visitor&) const final;
    };

    struct GetIndex : public ValExpr {
        ExprPtr idx;

        GetIndex(ExprPtr idx) :
            ValExpr(types::UINT32), idx(idx)
        {
            assert(idx->type == types::INDEX_PTR);
        }

        void Accept(Visitor&) const final;
    };

    struct Fetch : public ValExpr {
        ExprPtr reg;
        ExprPtr idx;

        Fetch(ExprPtr reg, ExprPtr idx) :
            ValExpr(DataType(reg->type.dtype.ptypes, true)),
            reg(reg), idx(idx)
        {
            assert(reg->type.isLStream());
            assert(idx->type == types::INDEX_PTR);
        }

        void Accept(Visitor&) const final;
    };

    struct Load : public ValExpr {
        ExprPtr ptr;

        Load(ExprPtr ptr) :
            ValExpr(DataType(ptr->type.dtype.ptypes, false)),
            ptr(ptr)
        {
            assert(ptr->type.dtype.is_ptr);
        }

        void Accept(Visitor&) const final;
    };

    struct Store : public Expr {
        ExprPtr reg;
        ExprPtr ptr;
        ExprPtr data;

        Store(ExprPtr reg, ExprPtr ptr, ExprPtr data) :
            Expr(reg->type), reg(reg), ptr(ptr), data(data)
        {}

        void Accept(Visitor&) const final;
    };

    struct Advance : public ValExpr {
        ExprPtr reg;
        ExprPtr idx;
        ExprPtr time;

        Advance(ExprPtr reg, ExprPtr idx, ExprPtr time) :
            ValExpr(types::INDEX_PTR), reg(reg), idx(idx), time(time)
        {
            assert(reg->type.isLStream());
            assert(idx->type == types::INDEX_PTR);
            assert(time->type == types::TIME);
        }

        void Accept(Visitor&) const final;
    };

    struct NextTime : public ValExpr {
        ExprPtr reg;
        ExprPtr idx;

        NextTime(ExprPtr reg, ExprPtr idx) :
            ValExpr(types::TIME), reg(reg), idx(idx)
        {
            assert(reg->type.isLStream());
            assert(idx->type == types::INDEX_PTR);
        }

        void Accept(Visitor&) const final;
    };

    struct GetStartIdx : public ValExpr {
        ExprPtr reg;

        GetStartIdx(ExprPtr reg) :
            ValExpr(types::INDEX_PTR), reg(reg)
        {
            assert(reg->type.isLStream());
        }

        void Accept(Visitor&) const final;
    };

    struct GetEndIdx : public ValExpr {
        ExprPtr reg;

        GetEndIdx(ExprPtr reg) :
            ValExpr(types::INDEX_PTR), reg(reg)
        {
            assert(reg->type.isLStream());
        }

        void Accept(Visitor&) const final;
    };

    struct CommitData : public Expr {
        ExprPtr reg;
        ExprPtr time;

        CommitData(ExprPtr reg, ExprPtr time) :
            Expr(reg->type), reg(reg), time(time)
        {
            assert(reg->type.isLStream());
            assert(time->type == types::TIME);
        }

        void Accept(Visitor&) const final;
    };

    struct CommitNull : public Expr {
        ExprPtr reg;
        ExprPtr time;

        CommitNull(ExprPtr reg, ExprPtr time) :
            Expr(reg->type), reg(reg), time(time)
        {
            assert(reg->type.isLStream());
            assert(time->type == types::TIME);
        }

        void Accept(Visitor&) const final;
    };

    struct AllocRegion : public Expr {
        ValExprPtr size;
        ExprPtr start_time;

        AllocRegion(Type type, ValExprPtr size, ExprPtr start_time) :
            Expr(type), size(size), start_time(start_time)
        {
            assert(type.isLStream());
            assert(size->type.dtype == types::UINT32);
            assert(start_time->type.dtype == types::TIME);
        }

        void Accept(Visitor&) const final;
    };

    struct MakeRegion : public Expr {
        ExprPtr reg;
        ExprPtr start_idx;
        ExprPtr end_idx;

        MakeRegion(ExprPtr reg, ExprPtr start_idx, ExprPtr end_idx) :
            Expr(reg->type), reg(reg), start_idx(start_idx), end_idx(end_idx)
        {
            assert(reg->type.isLStream());
            assert(start_idx->type == types::INDEX_PTR);
            assert(end_idx->type == types::INDEX_PTR);
        }

        void Accept(Visitor&) const final;
    };

    struct Loop : public Func {
        // Loop counter
        Timer t;

        // Indices
        vector<Indexer> idxs;

        // States
        map<SymPtr, SymPtr> state_bases;

        // loop condition
        PredPtr exit_cond;

        // Inner loops
        vector<shared_ptr<Loop>> inner_loops;

        Loop(string name, Type type) : Func(name, move(type)) {}
        Loop(SymPtr sym) : Loop(sym->name, sym->type) {}

        const string GetName() const override { return "loop_" + this->name; }

        void Accept(Visitor&) const final;
    };
    typedef shared_ptr<Loop> Looper;

} // namespace tilt

#endif // TILT_LOOP
