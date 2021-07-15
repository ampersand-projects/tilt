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
            assert(!type.is_valtype());
        }
    };

    struct AllocIndex : public ValNode {
        Expr init_idx;

        AllocIndex(Expr init_idx) :
            ValNode(types::INDEX_PTR), init_idx(init_idx)
        {
            assert(init_idx->type == types::INDEX_PTR);
        }

        void Accept(Visitor&) const final;
    };

    struct GetTime : public ValNode {
        Expr idx;

        GetTime(Expr idx) :
            ValNode(types::TIME), idx(idx)
        {
            assert(idx->type == types::INDEX_PTR);
        }

        void Accept(Visitor&) const final;
    };

    struct GetIndex : public ValNode {
        Expr idx;

        GetIndex(Expr idx) :
            ValNode(types::UINT32), idx(idx)
        {
            assert(idx->type == types::INDEX_PTR);
        }

        void Accept(Visitor&) const final;
    };

    struct Fetch : public ValNode {
        Expr reg;
        Expr idx;

        Fetch(Expr reg, Expr idx) :
            ValNode(DataType(reg->type.dtype.ptypes, true)),
            reg(reg), idx(idx)
        {
            assert(!reg->type.is_valtype());
            assert(idx->type == types::INDEX_PTR);
        }

        void Accept(Visitor&) const final;
    };

    struct Load : public ValNode {
        Expr ptr;

        Load(Expr ptr) :
            ValNode(DataType(ptr->type.dtype.ptypes, false)),
            ptr(ptr)
        {
            assert(ptr->type.dtype.is_ptr);
        }

        void Accept(Visitor&) const final;
    };

    struct Store : public ExprNode {
        Expr reg;
        Expr ptr;
        Expr data;

        Store(Expr reg, Expr ptr, Expr data) :
            ExprNode(reg->type), reg(reg), ptr(ptr), data(data)
        {}

        void Accept(Visitor&) const final;
    };

    struct Advance : public ValNode {
        Expr reg;
        Expr idx;
        Expr time;

        Advance(Expr reg, Expr idx, Expr time) :
            ValNode(types::INDEX_PTR), reg(reg), idx(idx), time(time)
        {
            assert(!reg->type.is_valtype());
            assert(idx->type == types::INDEX_PTR);
            assert(time->type == types::TIME);
        }

        void Accept(Visitor&) const final;
    };

    struct NextTime : public ValNode {
        Expr reg;
        Expr idx;

        NextTime(Expr reg, Expr idx) :
            ValNode(types::TIME), reg(reg), idx(idx)
        {
            assert(!reg->type.is_valtype());
            assert(idx->type == types::INDEX_PTR);
        }

        void Accept(Visitor&) const final;
    };

    struct GetStartIdx : public ValNode {
        Expr reg;

        GetStartIdx(Expr reg) :
            ValNode(types::INDEX_PTR), reg(reg)
        {
            assert(!reg->type.is_valtype());
        }

        void Accept(Visitor&) const final;
    };

    struct GetEndIdx : public ValNode {
        Expr reg;

        GetEndIdx(Expr reg) :
            ValNode(types::INDEX_PTR), reg(reg)
        {
            assert(!reg->type.is_valtype());
        }

        void Accept(Visitor&) const final;
    };

    struct CommitData : public ExprNode {
        Expr reg;
        Expr time;

        CommitData(Expr reg, Expr time) :
            ExprNode(reg->type), reg(reg), time(time)
        {
            assert(!reg->type.is_valtype());
            assert(time->type == types::TIME);
        }

        void Accept(Visitor&) const final;
    };

    struct CommitNull : public ExprNode {
        Expr reg;
        Expr time;

        CommitNull(Expr reg, Expr time) :
            ExprNode(reg->type), reg(reg), time(time)
        {
            assert(!reg->type.is_valtype());
            assert(time->type == types::TIME);
        }

        void Accept(Visitor&) const final;
    };

    struct AllocRegion : public ExprNode {
        Val size;
        Expr start_time;

        AllocRegion(Type type, Val size, Expr start_time) :
            ExprNode(type), size(size), start_time(start_time)
        {
            assert(!type.is_valtype());
            assert(size->type.dtype == types::UINT32);
            assert(start_time->type.dtype == types::TIME);
        }

        void Accept(Visitor&) const final;
    };

    struct MakeRegion : public ExprNode {
        Expr reg;
        Expr start_idx;
        Expr end_idx;

        MakeRegion(Expr reg, Expr start_idx, Expr end_idx) :
            ExprNode(reg->type), reg(reg), start_idx(start_idx), end_idx(end_idx)
        {
            assert(!reg->type.is_valtype());
            assert(start_idx->type == types::INDEX_PTR);
            assert(end_idx->type == types::INDEX_PTR);
        }

        void Accept(Visitor&) const final;
    };

    struct Loop : public FuncNode {
        // Loop counter
        Timer t;

        // Indices
        vector<Indexer> idxs;

        // States
        map<Sym, Sym> state_bases;

        // loop condition
        Pred exit_cond;

        // Inner loops
        vector<shared_ptr<Loop>> inner_loops;

        Loop(string name, Type type) : FuncNode(name, move(type)) {}
        Loop(Sym sym) : Loop(sym->name, sym->type) {}

        const string GetName() const override { return "loop_" + this->name; }

        void Accept(Visitor&) const final;
    };
    typedef shared_ptr<Loop> Looper;

} // namespace tilt

#endif // TILT_LOOP
