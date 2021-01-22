#ifndef TILL_OP
#define TILL_OP

#include <array>
#include <memory>

namespace till {

    template<std::size_t N>
    class Op {
    private:
        std::array<std::shared_ptr<Op>, N> inputs;

    public:

    }; // class Op
} // namespace till

#endif // TILL_OP