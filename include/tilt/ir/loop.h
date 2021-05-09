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
        Index(string name) : Symbol(name, types::INDEX) {}
    };
    typedef shared_ptr<Index> Indexer;

    struct Region : public Symbol {
        Region(string name, Type type) : Symbol(name, move(type)) {}
    };
    typedef shared_ptr<Region> RegPtr;

    struct GetTime : public ValExpr {
        ExprPtr idx;

        GetTime(ExprPtr idx) : ValExpr(types::TIME), idx(idx) {}

        void Accept(Visitor&) const final;
    };

    struct Fetch : public ValExpr {
        ExprPtr reg;
        ExprPtr idx;

        Fetch(RegPtr reg, ExprPtr idx) :
            ValExpr(reg->type.dtype), reg(reg), idx(idx)
        {}

        void Accept(Visitor&) const final;
    };

    struct Advance : public ValExpr {
        ExprPtr reg;
        ExprPtr idx;
        ExprPtr time;

        Advance(ExprPtr reg, ExprPtr idx, ExprPtr time) :
            ValExpr(types::INDEX), reg(reg), idx(idx), time(time)
        {}

        void Accept(Visitor&) const final;
    };

    struct Next : public ValExpr {
        ExprPtr reg;
        ExprPtr idx;

        Next(ExprPtr reg, ExprPtr idx) :
            ValExpr(types::INDEX), reg(reg), idx(idx)
        {}

        void Accept(Visitor&) const final;
    };

    struct Stmt : public ASTNode {
    };
    typedef shared_ptr<Stmt> StmtPtr;

    struct CommitData : public Stmt {
        ExprPtr region;
        ExprPtr time;
        ExprPtr data;

        CommitData(ExprPtr region, ExprPtr time, ExprPtr data) :
            region(region), time(time), data(data)
        {
            //assert(data->type.dtype == region->type.dtype);
        }

        void Accept(Visitor&) const final;
    };

    struct CommitNull : public Stmt {
        ExprPtr region;
        ExprPtr time;

        CommitNull(ExprPtr region, ExprPtr time) :
            region(region), time(time)
        {}

        void Accept(Visitor&) const final;
    };

    struct Block : public Stmt {
        vector<StmtPtr> stmts;

        void Accept(Visitor&) const final;
    };
    typedef shared_ptr<Block> BlockPtr;

    struct Loop : public Stmt {
        string name;

        // Arguments
        Timer t_start, t_end;
        RegPtr out_reg;
        vector<RegPtr> in_regs;

        // States
        Timer t_cur, t_prev;
        map<Indexer, RegPtr> idx_map;

        // Incrementor
        ExprPtr next_t;

        // Update indices
        map<Indexer, ExprPtr> idx_update;

        // Set vars
        map<SymPtr, ExprPtr> vars;

        // Predicate
        ExprPtr pred;
        StmtPtr true_body;
        StmtPtr false_body;

        Loop(string name) : name(name) {}

        void Accept(Visitor&) const final;
    };
    typedef shared_ptr<Loop> Looper;

} // namespace tilt

#endif // TILT_LOOP