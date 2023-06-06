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

struct TimeNode : public Symbol {
    explicit TimeNode(string name) : Symbol(name, Type(types::TIME)) {}
};
typedef shared_ptr<TimeNode> Time;

struct Fetch : public ValNode {
    Expr reg;
    Expr time;

    Fetch(Expr reg, Expr time) :
        ValNode(reg->type.dtype.ptr()), reg(reg), time(time)
    {
        ASSERT(!reg->type.is_val());
        ASSERT(time->type.dtype == types::TIME);
    }

    void Accept(Visitor&) const final;
};

struct Read : public ValNode {
    Expr ptr;

    explicit Read(Expr ptr) : ValNode(ptr->type.dtype.deref()), ptr(ptr) {}

    void Accept(Visitor&) const final;
};

struct Write : public ExprNode {
    Expr reg;
    Expr ptr;
    Expr data;

    Write(Expr reg, Expr ptr, Expr data) :
        ExprNode(reg->type), reg(reg), ptr(ptr), data(data)
    {
        ASSERT(ptr->type.dtype.is_ptr());
    }

    void Accept(Visitor&) const final;
};

struct GetCkpt : public ValNode {
    Expr reg;
    Expr time;

    GetCkpt(Expr reg, Expr time) :
        ValNode(types::TIME), reg(reg), time(time)
    {
        ASSERT(!reg->type.is_val());
        ASSERT(time->type.dtype == types::TIME);
    }

    void Accept(Visitor&) const final;
};

struct GetStartTime : public ValNode {
    Expr reg;

    explicit GetStartTime(Expr reg) : ValNode(types::TIME), reg(reg)
    {
        ASSERT(!reg->type.is_val());
    }

    void Accept(Visitor&) const final;
};

struct GetEndTime : public ValNode {
    Expr reg;

    explicit GetEndTime(Expr reg) : ValNode(types::TIME), reg(reg)
    {
        ASSERT(!reg->type.is_val());
    }

    void Accept(Visitor&) const final;
};

struct CommitData : public ExprNode {
    Expr reg;
    Expr time;

    CommitData(Expr reg, Expr time) :
        ExprNode(reg->type), reg(reg), time(time)
    {
        ASSERT(!reg->type.is_val());
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
        ASSERT(!reg->type.is_val());
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
        ASSERT(!type.is_val());
        ASSERT(size->type.dtype == types::TIME);
        ASSERT(start_time->type.dtype == types::TIME);
    }

    void Accept(Visitor&) const final;
};

struct MakeRegion : public ExprNode {
    Expr reg;
    Expr st;
    Expr et;

    MakeRegion(Expr reg, Expr st, Expr et) :
        ExprNode(reg->type), reg(reg), st(st), et(et)
    {
        ASSERT(!reg->type.is_val());
        ASSERT(st->type.dtype == types::TIME);
        ASSERT(et->type.dtype == types::TIME);
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

struct LoopNode : public FuncNode {
    // Loop counter
    Time t;

    // States
    map<Sym, Sym> state_bases;

    // loop condition
    Expr exit_cond;

    // Inner loops
    vector<shared_ptr<LoopNode>> inner_loops;

    LoopNode(string name, Type type) : FuncNode(name, move(type)) {}
    explicit LoopNode(Sym sym) : LoopNode(sym->name, sym->type) {}

    const string get_name() const override { return "loop_" + this->name; }

    void Accept(Visitor&) const final;
};
typedef shared_ptr<LoopNode> Loop;

}  // namespace tilt

#endif  // INCLUDE_TILT_IR_LOOP_H_
