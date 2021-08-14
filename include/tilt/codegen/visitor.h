#ifndef INCLUDE_TILT_CODEGEN_VISITOR_H_
#define INCLUDE_TILT_CODEGEN_VISITOR_H_

#include "tilt/ir/expr.h"
#include "tilt/ir/lstream.h"
#include "tilt/ir/op.h"
#include "tilt/ir/loop.h"

namespace tilt {

class Visitor {
public:
    /**
     * TiLT IR
     */
    virtual void Visit(const Symbol&) = 0;
    virtual void Visit(const Out&) = 0;
    virtual void Visit(const Beat&) = 0;
    virtual void Visit(const Call&) = 0;
    virtual void Visit(const Read&) = 0;
    virtual void Visit(const IfElse&) = 0;
    virtual void Visit(const Select&) = 0;
    virtual void Visit(const Get&) = 0;
    virtual void Visit(const New&) = 0;
    virtual void Visit(const Exists&) = 0;
    virtual void Visit(const ConstNode&) = 0;
    virtual void Visit(const Cast&) = 0;
    virtual void Visit(const NaryExpr&) = 0;
    virtual void Visit(const SubLStream&) = 0;
    virtual void Visit(const Element&) = 0;
    virtual void Visit(const OpNode&) = 0;
    virtual void Visit(const Reduce&) = 0;

    /**
     * Loop IR
     */
    virtual void Visit(const Fetch&) = 0;
    virtual void Visit(const Write&) = 0;
    virtual void Visit(const Advance&) = 0;
    virtual void Visit(const GetCkpt&) = 0;
    virtual void Visit(const GetStartIdx&) = 0;
    virtual void Visit(const GetEndIdx&) = 0;
    virtual void Visit(const GetStartTime&) = 0;
    virtual void Visit(const GetEndTime&) = 0;
    virtual void Visit(const CommitData&) = 0;
    virtual void Visit(const CommitNull&) = 0;
    virtual void Visit(const AllocRegion&) = 0;
    virtual void Visit(const MakeRegion&) = 0;
    virtual void Visit(const LoopNode&) = 0;
};

}  // namespace tilt

#endif  // INCLUDE_TILT_CODEGEN_VISITOR_H_
