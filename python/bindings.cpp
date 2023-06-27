#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include <vector>
#include <memory>
#include <string>
#include <iostream>

#include "tilt/base/type.h"
#include "tilt/builder/tilder.h"
#include "tilt/ir/node.h"
#include "tilt/ir/expr.h"
#include "tilt/ir/lstream.h"
#include "tilt/ir/op.h"

#include "tilt/pass/printer.h"
#include "tilt/pass/codegen/loopgen.h"

using namespace std;
using namespace tilt;
using namespace tilt::tilder;

namespace py = pybind11;

void print_IR(Op query_op)
{
    cout << "TiLT IR:" << endl;
    cout << IRPrinter::Build(query_op) << endl;

    auto query_op_sym = _sym("query", query_op);
    auto loop = LoopGen::Build(query_op_sym, query_op.get());

    cout << "Loop IR:" << endl;
    cout << IRPrinter::Build(loop);
}

#define REGISTER_NOINIT_CLASS(CLASS, PARENT, MODULE, NAME) \
    py::class_<CLASS, shared_ptr<CLASS>, PARENT>(MODULE, NAME);

#define REGISTER_INIT_CLASS(CLASS, PARENT, MODULE, NAME, ...) \
    py::class_<CLASS, shared_ptr<CLASS>, PARENT>(MODULE, NAME) \
        .def(py::init<__VA_ARGS__>());

#define REGISTER_UNARY_EXPR(MODULE, EXPR, NAME) \
    REGISTER_INIT_CLASS(EXPR, UnaryExpr, MODULE, NAME, Expr)

#define REGISTER_BINARY_EXPR(MODULE, EXPR, NAME) \
    REGISTER_INIT_CLASS(EXPR, BinaryExpr, MODULE, NAME, Expr, Expr)

#define REGISTER_NOPARENT_INIT_CLASS(CLASS, MODULE, NAME, ...) \
    py::class_<CLASS, shared_ptr<CLASS>>(MODULE, NAME) \
        .def(py::init<__VA_ARGS__>());

PYBIND11_MODULE(pytilt, m) {
    /* Structures related to TiLT typing */
    py::enum_<BaseType>(m, "BaseType")
        .value("bool", BaseType::BOOL)
        .value("i8", BaseType::INT8)
        .value("i16", BaseType::INT16)
        .value("i32", BaseType::INT32)
        .value("i64", BaseType::INT64)
        .value("u8", BaseType::UINT8)
        .value("u16", BaseType::UINT16)
        .value("u32", BaseType::UINT32)
        .value("u64", BaseType::UINT64)
        .value("f32", BaseType::FLOAT32)
        .value("f64", BaseType::FLOAT64)
        .value("struct", BaseType::STRUCT)
        .value("ptr", BaseType::PTR)
        .value("t", BaseType::TIME)
        .value("idx", BaseType::INDEX)
        .value("ival", BaseType::IVAL);

    py::class_<DataType>(m, "DataType")
        .def(py::init<BaseType, vector<DataType>, size_t>(),
              py::arg("btype"),
              py::arg("dtypes") = vector<DataType>{},
              py::arg("size") = 0)
        .def("str", &DataType::str);

    py::class_<Iter>(m, "Iter")
        .def(py::init<int64_t, int64_t>(),
              py::arg("offset") = 0,
              py::arg("period") = 0)
        .def("str", &Iter::str);

    py::class_<Type>(m, "Type")
        .def(py::init<DataType, Iter>());

    /* ExprNode and Derived Structures Declarations */
    py::class_<ExprNode, Expr>(m, "expr")
        .def("getType",
              [](Expr expr) {
                    return expr->type;
              });
    REGISTER_NOINIT_CLASS(ValNode, ExprNode, m, "val")

    /* Symbol Definition */
    py::class_<Symbol, Sym, ExprNode>(m, "sym")
        .def(py::init<string, Type>())
        .def(py::init<string, Expr>());

    /* Element/Substream/Windowing and Related Type Bindings */
    REGISTER_NOPARENT_INIT_CLASS(Point, m, "point", int64_t)
    REGISTER_NOPARENT_INIT_CLASS(Window, m, "window", int64_t, int64_t)
    REGISTER_INIT_CLASS(Element, ValNode, m, "elem", Sym, Point)
    REGISTER_NOINIT_CLASS(LStream, ExprNode, m, "lstream")
    REGISTER_INIT_CLASS(SubLStream, LStream, m, "sublstream", Sym, Window)

    /* Constant Expressions */
    REGISTER_INIT_CLASS(ConstNode, ValNode, m, "const", BaseType, double)

    /* Nary Expressions */
    REGISTER_NOINIT_CLASS(NaryExpr, ValNode, m, "nary_expr")
    REGISTER_NOINIT_CLASS(UnaryExpr, NaryExpr, m, "unary_expr")
    REGISTER_NOINIT_CLASS(BinaryExpr, NaryExpr, m, "binary_expr")

    /* Arithmetic Expressions */
    REGISTER_BINARY_EXPR(m, Add, "add")
    REGISTER_BINARY_EXPR(m, Sub, "sub")
    REGISTER_BINARY_EXPR(m, Mul, "mul")
    REGISTER_BINARY_EXPR(m, Div, "div")
    REGISTER_BINARY_EXPR(m, Max, "max")
    REGISTER_BINARY_EXPR(m, Min, "min")
    REGISTER_UNARY_EXPR(m, Abs, "abs")
    REGISTER_UNARY_EXPR(m, Neg, "neg")
    REGISTER_BINARY_EXPR(m, Mod, "mod")
    REGISTER_UNARY_EXPR(m, Sqrt, "sqrt")
    REGISTER_BINARY_EXPR(m, Pow, "pow")
    REGISTER_UNARY_EXPR(m, Ceil, "ceil")
    REGISTER_UNARY_EXPR(m, Floor, "floor")
    REGISTER_BINARY_EXPR(m, LessThan, "lt")
    REGISTER_BINARY_EXPR(m, LessThanEqual, "lte")
    REGISTER_BINARY_EXPR(m, GreaterThan, "gt")
    REGISTER_BINARY_EXPR(m, GreaterThanEqual, "gte")
    REGISTER_BINARY_EXPR(m, Equals, "eq")

    /* Logical Expressions */
    REGISTER_INIT_CLASS(Exists, ValNode, m, "exists", Sym)
    REGISTER_UNARY_EXPR(m, Not, "not")
    REGISTER_BINARY_EXPR(m, And, "and")
    REGISTER_BINARY_EXPR(m, Or, "or")

    /* Misc Expressions */
    REGISTER_INIT_CLASS(Get, ValNode, m, "get", Expr, size_t)
    REGISTER_INIT_CLASS(New, ValNode, m, "new", vector<Expr>)
    REGISTER_INIT_CLASS(IfElse, ExprNode, m, "ifelse", Expr, Expr, Expr)
    REGISTER_INIT_CLASS(Cast, ValNode, m, "cast", DataType, Expr)

    /* Reduction Node */
    REGISTER_INIT_CLASS(Reduce, ValNode, m, "reduce", Sym, Val, AccTy)

    /* Operator Definition */
    py::class_<OpNode, Op>(m, "op")
        .def(py::init<Iter, Params, SymTable, Expr, Sym, Aux>(),
              py::arg("iter"),
              py::arg("inputs"),
              py::arg("syms"),
              py::arg("pred"),
              py::arg("output"),
              py::arg("aux") = map<Sym, Sym>{});

    /* Temp */
    m.def("print_IR", &print_IR);
}
