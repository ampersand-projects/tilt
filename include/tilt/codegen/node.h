#ifndef TILT_NODE
#define TILT_NODE

#include <memory>

using namespace std;

namespace tilt {

    class Visitor;

    struct ASTNode {
        virtual void Accept(Visitor&) const = 0;

        virtual ~ASTNode(){}
    };
    typedef shared_ptr<ASTNode> ASTPtr;

} // namespace tilt

#endif // TILT_NODE