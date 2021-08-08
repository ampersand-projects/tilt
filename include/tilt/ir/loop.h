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
    explicit Index(string name) : Symbol(name, Type(types::INDEX)) {}
};
typedef shared_ptr<Index> Indexer;

struct Region : public Symbol {
    Region(string name, Type type) : Symbol(name, move(type))
    {
        ASSERT(!type.is_valtype());
    }
};

struct Fetch : public ValNode {
    Expr reg;
    Expr time;
    Expr idx;

    Fetch(Expr reg, Expr time, Expr idx) :
        ValNode(DataType(BaseType::PTR, {reg->type.dtype})), reg(reg), time(time), idx(idx)
    {
        ASSERT(!reg->type.is_valtype());
        ASSERT(time->type.dtype == types::TIME);
        ASSERT(idx->type.dtype == types::INDEX);
    }

    void Accept(Visitor&) const final;
};

struct Load : public ValNode {
    Expr ptr;

    explicit Load(Expr ptr) :
        ValNode(DataType(ptr->type.dtype.dtypes[0])), ptr(ptr)
    {
        ASSERT(ptr->type.dtype.is_ptr());
    }

    void Accept(Visitor&) const final;
};

struct Store : public ExprNode {
    Expr reg;
    Expr ptr;
    Expr data;

    Store(Expr reg, Expr ptr, Expr data) :
        ExprNode(reg->type), reg(reg), ptr(ptr), data(data)
    {
        ASSERT(ptr->type.dtype.is_ptr());
    }

    void Accept(Visitor&) const final;
};

struct Advance : public ValNode {
    Expr reg;
    Expr idx;
    Expr time;

    Advance(Expr reg, Expr idx, Expr time) :
        ValNode(types::INDEX), reg(reg), idx(idx), time(time)
    {
        ASSERT(!reg->type.is_valtype());
        ASSERT(idx->type.dtype == types::INDEX);
        ASSERT(time->type.dtype == types::TIME);
    }

    void Accept(Visitor&) const final;
};

struct GetCkpt : public ValNode {
    Expr reg;
    Expr time;
    Expr idx;

    GetCkpt(Expr reg, Expr time, Expr idx) :
        ValNode(types::TIME), reg(reg), time(time), idx(idx)
    {
        ASSERT(!reg->type.is_valtype());
        ASSERT(time->type.dtype == types::TIME);
        ASSERT(idx->type.dtype == types::INDEX);
    }

    void Accept(Visitor&) const final;
};

struct GetStartIdx : public ValNode {
    Expr reg;

    explicit GetStartIdx(Expr reg) : ValNode(types::INDEX), reg(reg)
    {
        ASSERT(!reg->type.is_valtype());
    }

    void Accept(Visitor&) const final;
};

struct GetEndIdx : public ValNode {
    Expr reg;

    explicit GetEndIdx(Expr reg) : ValNode(types::INDEX), reg(reg)
    {
        ASSERT(!reg->type.is_valtype());
    }

    void Accept(Visitor&) const final;
};

struct CommitData : public ExprNode {
    Expr reg;
    Expr time;

    CommitData(Expr reg, Expr time) :
        ExprNode(reg->type), reg(reg), time(time)
    {
        ASSERT(!reg->type.is_valtype());
        ASSERT(time->type.dtype == types::TIME);
    }

    void Accept(Visitor&) const final;
};

struct CommitNull : public ExprNode {
    Expr reg;
    Expr time;

    CommitNull(Expr reg, Expr time) :
        ExprNode(reg->type), reg(reg), time(time)
    {
        ASSERT(!reg->type.is_valtype());
        ASSERT(time->type.dtype == types::TIME);
    }

    void Accept(Visitor&) const final;
};

struct AllocRegion : public ExprNode {
    Val size;
    Expr start_time;

    AllocRegion(Type type, Val size, Expr start_time) :
        ExprNode(type), size(size), start_time(start_time)
    {
        ASSERT(!type.is_valtype());
        ASSERT(size->type.dtype == types::INDEX);
        ASSERT(start_time->type.dtype == types::TIME);
    }

    void Accept(Visitor&) const final;
};

struct MakeRegion : public ExprNode {
    Expr reg;
    Expr st;
    Expr si;
    Expr et;
    Expr ei;

    MakeRegion(Expr reg, Expr st, Expr si, Expr et, Expr ei) :
        ExprNode(reg->type), reg(reg), st(st), si(si), et(et), ei(ei)
    {
        ASSERT(!reg->type.is_valtype());
        ASSERT(st->type.dtype == types::TIME);
        ASSERT(si->type.dtype == types::INDEX);
        ASSERT(et->type.dtype == types::TIME);
        ASSERT(ei->type.dtype == types::INDEX);
    }

    void Accept(Visitor&) const final;
};

struct IfElse : public ExprNode {
    Expr cond;
    Expr true_body;
    Expr false_body;

    IfElse(Expr cond, Expr true_body, Expr false_body) :
        ExprNode(true_body->type), cond(cond), true_body(true_body), false_body(false_body)
    {
        ASSERT(cond->type.dtype == types::BOOL);
        ASSERT(true_body->type.dtype == false_body->type.dtype);
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
