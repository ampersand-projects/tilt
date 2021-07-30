#ifndef INCLUDE_TILT_IR_LOOP_H_
#define INCLUDE_TILT_IR_LOOP_H_

#include <map>
#include <memory>
#include <vector>
#include <utility>
#include <string>

#include "tilt/ir/node.h"
#include "tilt/ir/expr.h"

using namespace std;

namespace tilt {

struct Time : public Symbol {
    explicit Time(string name) : Symbol(name, Type(types::TIME)) {}
};
typedef shared_ptr<Time> Timer;

struct Index : public Symbol {
    explicit Index(string name) : Symbol(name, Type(types::INDEX_PTR)) {}
};
typedef shared_ptr<Index> Indexer;

struct Region : public Symbol {
    Region(string name, Type type) : Symbol(name, move(type))
    {
        CHECK(!type.is_valtype());
    }
};

struct AllocIndex : public ValNode {
    Expr init_idx;

    explicit AllocIndex(Expr init_idx) : ValNode(types::INDEX_PTR), init_idx(init_idx)
    {
        CHECK(init_idx->type.dtype == types::INDEX_PTR);
    }

    void Accept(Visitor&) const final;
};

struct GetTime : public ValNode {
    Expr idx;

    explicit GetTime(Expr idx) : ValNode(types::TIME), idx(idx)
    {
        CHECK(idx->type.dtype == types::INDEX_PTR);
    }

    void Accept(Visitor&) const final;
};

struct GetIndex : public ValNode {
    Expr idx;

    explicit GetIndex(Expr idx) : ValNode(types::UINT32), idx(idx)
    {
        CHECK(idx->type.dtype == types::INDEX_PTR);
    }

    void Accept(Visitor&) const final;
};

struct Fetch : public ValNode {
    Expr reg;
    Expr idx;

    Fetch(Expr reg, Expr idx) :
        ValNode(DataType(BaseType::PTR, {reg->type.dtype})), reg(reg), idx(idx)
    {
        CHECK(!reg->type.is_valtype());
        CHECK(idx->type.dtype == types::INDEX_PTR);
    }

    void Accept(Visitor&) const final;
};

struct Load : public ValNode {
    Expr ptr;

    explicit Load(Expr ptr) : ValNode(DataType(ptr->type.dtype.dtypes[0])), ptr(ptr)
    {
        CHECK(ptr->type.dtype.is_ptr());
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
        CHECK(!reg->type.is_valtype());
        CHECK(idx->type.dtype == types::INDEX_PTR);
        CHECK(time->type.dtype == types::TIME);
    }

    void Accept(Visitor&) const final;
};

struct NextTime : public ValNode {
    Expr reg;
    Expr idx;

    NextTime(Expr reg, Expr idx) :
        ValNode(types::TIME), reg(reg), idx(idx)
    {
        CHECK(!reg->type.is_valtype());
        CHECK(idx->type.dtype == types::INDEX_PTR);
    }

    void Accept(Visitor&) const final;
};

struct GetStartIdx : public ValNode {
    Expr reg;

    explicit GetStartIdx(Expr reg) : ValNode(types::INDEX_PTR), reg(reg)
    {
        CHECK(!reg->type.is_valtype());
    }

    void Accept(Visitor&) const final;
};

struct GetEndIdx : public ValNode {
    Expr reg;

    explicit GetEndIdx(Expr reg) : ValNode(types::INDEX_PTR), reg(reg)
    {
        CHECK(!reg->type.is_valtype());
    }

    void Accept(Visitor&) const final;
};

struct CommitData : public ExprNode {
    Expr reg;
    Expr time;

    CommitData(Expr reg, Expr time) :
        ExprNode(reg->type), reg(reg), time(time)
    {
        CHECK(!reg->type.is_valtype());
        CHECK(time->type.dtype == types::TIME);
    }

    void Accept(Visitor&) const final;
};

struct CommitNull : public ExprNode {
    Expr reg;
    Expr time;

    CommitNull(Expr reg, Expr time) :
        ExprNode(reg->type), reg(reg), time(time)
    {
        CHECK(!reg->type.is_valtype());
        CHECK(time->type.dtype == types::TIME);
    }

    void Accept(Visitor&) const final;
};

struct AllocRegion : public ExprNode {
    Val size;
    Expr start_time;

    AllocRegion(Type type, Val size, Expr start_time) :
        ExprNode(type), size(size), start_time(start_time)
    {
        CHECK(!type.is_valtype());
        CHECK(size->type.dtype == types::UINT32);
        CHECK(start_time->type.dtype == types::TIME);
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
        CHECK(!reg->type.is_valtype());
        CHECK(start_idx->type.dtype == types::INDEX_PTR);
        CHECK(end_idx->type.dtype == types::INDEX_PTR);
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
    Expr exit_cond;

    // Inner loops
    vector<shared_ptr<Loop>> inner_loops;

    Loop(string name, Type type) : FuncNode(name, move(type)) {}
    explicit Loop(Sym sym) : Loop(sym->name, sym->type) {}

    const string GetName() const override { return "loop_" + this->name; }

    void Accept(Visitor&) const final;
};
typedef shared_ptr<Loop> Looper;

}  // namespace tilt

#endif  // INCLUDE_TILT_IR_LOOP_H_
