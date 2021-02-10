#ifndef TILT_LLVMGEN
#define TILT_LLVMGEN

#include "tilt/codegen/visitor.h"

namespace tilt
{

    class LLVMGen : public Visitor {
    public:
        void Visit(const Symbol&) override;
        void Visit(const Lambda&) override;
        void Visit(const Exists&) override;
        void Visit(const Equals&) override;
        void Visit(const Not&) override;
        void Visit(const And&) override;
        void Visit(const Or&) override;
        void Visit(const IConst&) override;
        void Visit(const UConst&) override;
        void Visit(const FConst&) override;
        void Visit(const BConst&) override;
        void Visit(const CConst&) override;
        void Visit(const Add&) override;
        void Visit(const Now&) override;

        void Visit(const SubLStream&) override;
        void Visit(const Element&) override;

        void Visit(const Op&) override;
        void Visit(const Sum&) override;
    };

} // namespace tilt


#endif // TILT_LLVMGEN
