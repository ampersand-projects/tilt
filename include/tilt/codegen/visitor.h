#ifndef TILT_VISITOR
#define TILT_VISITOR

#include "tilt/ir/expr.h"
#include "tilt/ir/lstream.h"
#include "tilt/ir/op.h"
#include "tilt/ir/loop.h"

namespace tilt
{

    class Visitor {
    public:
        /**
         * TiLT IR
         */
        virtual void Visit(const Symbol&) = 0;
        virtual void Visit(const Lambda&) = 0;
        virtual void Visit(const Exists&) = 0;
        virtual void Visit(const Equals&) = 0;
        virtual void Visit(const Not&) = 0;
        virtual void Visit(const And&) = 0;
        virtual void Visit(const Or&) = 0;
        virtual void Visit(const IConst&) = 0;
        virtual void Visit(const UConst&) = 0;
        virtual void Visit(const FConst&) = 0;
        virtual void Visit(const BConst&) = 0;
        virtual void Visit(const CConst&) = 0;
        virtual void Visit(const TConst&) = 0;
        virtual void Visit(const Add&) = 0;
        virtual void Visit(const Sub&) = 0;
        virtual void Visit(const Max&) = 0;
        virtual void Visit(const Min&) = 0;
        virtual void Visit(const Now&) = 0;
        virtual void Visit(const True&) = 0;
        virtual void Visit(const False&) = 0;
        virtual void Visit(const LessThan&) = 0;
        virtual void Visit(const LessThanEqual&) = 0;

        virtual void Visit(const SubLStream&) = 0;
        virtual void Visit(const Element&) = 0;

        virtual void Visit(const Op&) = 0;
        virtual void Visit(const AggExpr&) = 0;

        /**
         * Loop IR
         */
        virtual void Visit(const GetTime&) = 0;
        virtual void Visit(const Fetch&) = 0;
        virtual void Visit(const Advance&) = 0;
        virtual void Visit(const Next&) = 0;
        virtual void Visit(const CommitData&) = 0;
        virtual void Visit(const CommitNull&) = 0;
        virtual void Visit(const Block&) = 0;
        virtual void Visit(const Loop&) = 0;
    };

} // namespace tilt

#endif // TILT_VISITOR