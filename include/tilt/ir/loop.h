#ifndef TILT_LOOP
#define TILT_LOOP

#include "tilt/codegen/node.h"
#include "tilt/ir/expr.h"
#include "tilt/ir/lstream.h"

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
        ExprPtr idx;

        AllocIndex(ExprPtr idx) :
            ValExpr(types::INDEX_PTR), idx(idx)
        {
            assert(idx->type == types::INDEX_PTR);
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

    struct Fetch : public ValExpr {
        ExprPtr reg;
        ExprPtr idx;

        Fetch(ExprPtr reg, ExprPtr idx) :
            ValExpr(move(DataType(reg->type.dtype.ptypes, true))),
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
            ValExpr(move(DataType(ptr->type.dtype.ptypes, false))),
            ptr(ptr)
        {
            assert(ptr->type.dtype.is_ptr);
        }

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

    struct Next : public ValExpr {
        ExprPtr reg;
        ExprPtr idx;

        Next(ExprPtr reg, ExprPtr idx) :
            ValExpr(types::INDEX_PTR), reg(reg), idx(idx)
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

    struct CommitData : public Expr {
        ExprPtr reg;
        ExprPtr time;
        ExprPtr data;

        CommitData(ExprPtr reg, ExprPtr time, ExprPtr data) :
            Expr(reg->type), reg(reg), time(time), data(data)
        {
            assert(reg->type.isLStream());
            assert(time->type == types::TIME);
            assert(data->type.dtype == reg->type.dtype);
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

        AllocRegion(Type type, ValExprPtr size) :
            Expr(type), size(size)
        {
            assert(type.isLStream());
            assert(size->type.dtype == types::TIME);
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

    struct Loop : public Expr {
        struct LoopState {
            SymPtr base;
            ExprPtr init;
            ExprPtr update;
        };

        // Identifier
        string name;

        // Arguments
        vector<SymPtr> inputs;

        // Loop counter
        Timer t;

        // Indices
        vector<Indexer> idxs;

        // Output
        RegPtr output;

        // States
        map<SymPtr, LoopState> states;

        // loop condition
        PredPtr exit_cond;

        // Local symbols
        SymTable syms;

        Loop(string name, Type type) : Expr(type), name(name) {}
        Loop(SymPtr sym) : Expr(sym->type), name(sym->name) {}

        const string GetName() const { return "loop_" + this->name; }

        void Accept(Visitor&) const final;
    };
    typedef shared_ptr<Loop> Looper;

    struct Call : public Expr {
        Looper loop;
        vector<ExprPtr> args;

        Call(Looper loop, vector<ExprPtr> args) :
            Expr(loop->type), loop(loop), args(move(args))
        {}

        void Accept(Visitor&) const final;
    };

} // namespace tilt

#endif // TILT_LOOP