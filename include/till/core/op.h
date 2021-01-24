#ifndef TILL_OP
#define TILL_OP

#include "till/core/expr.h"
#include "till/core/lstream.h"

using namespace std;

namespace till {

    template <typename>
    struct isLStream : public std::false_type { };
    template <typename T>
    struct isLStream<LStream<T>> : public std::true_type { };

    template <typename>
    struct isElement : public std::false_type { };
    template <typename T>
    struct isElement<Element<T>> : public std::true_type { };

    template<typename T>
    concept op_input_type = isLStream<T>::value || isElement<T>::value;

    template<typename DataType, op_input_type... Is>
    class OpBase : public LStream<DataType> {
    private:
        tuple<Is...> inputs;
    protected:
        OpBase(string name, TimeLine timeline, Is... inputs) :
            LStream<DataType>{ forward<string>(name), forward<TimeLine>(timeline) },
            inputs(move(make_tuple(inputs...)))
        {}
    }; // class OpBase

    template<typename DataType, typename... Is>
    class ElementOp : public OpBase<DataType, Element<Is>...> {
    private:
        Expr<DataType> body;
    }; // class ElementOp

    template<typename DataType, typename I>
    class AggregateOp : public OpBase<DataType, LStream<I>> {
    private:
        Expr<DataType> body;
    }; // class AggregateOp

    template<typename DataType, typename... Is>
    class LStreamOp : public OpBase<DataType, LStream<Is>...> {
    private:
        OpBase<DataType> body;
    }; // class LStreamOp

} // namespace till::op

#endif // TILL_OP