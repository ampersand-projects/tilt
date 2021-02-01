#ifndef TILL_NODE
#define TILL_NODE

namespace till {

    class Visitor;

    struct ASTNode {
        virtual void Accept(Visitor&) const = 0;
    };

} // namespace till

#endif // TILL_NODE