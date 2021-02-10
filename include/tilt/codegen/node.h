#ifndef TILT_NODE
#define TILT_NODE

namespace tilt {

    class Visitor;

    struct ASTNode {
        virtual void Accept(Visitor&) const = 0;
    };

} // namespace tilt

#endif // TILT_NODE