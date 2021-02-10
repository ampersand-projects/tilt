#ifndef TILT_VISITOR
#define TILT_VISITOR

#include "tilt/ir/expr.h"
#include "tilt/ir/lstream.h"
#include "tilt/ir/op.h"

namespace tilt
{

    class Visitor {
    public:
        virtual void Visit(const Symbol&) = 0;
        virtual void Visit(const Lambda&) = 0;
        virtual void Visit(const IConst&) = 0;
        virtual void Visit(const UConst&) = 0;
        virtual void Visit(const FConst&) = 0;
        virtual void Visit(const BConst&) = 0;
        virtual void Visit(const CConst&) = 0;
        virtual void Visit(const Add&) = 0;

        virtual void Visit(const SubLStream&) = 0;
        virtual void Visit(const Element&) = 0;

        virtual void Visit(const Op&) = 0;
        virtual void Visit(const Sum&) = 0;
    };

} // namespace tilt

#endif // TILT_VISITOR