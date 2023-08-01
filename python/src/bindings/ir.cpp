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

#define REGISTER_CLASS(CLASS, PARENT, MODULE, NAME, ...) \
    py::class_<CLASS, shared_ptr<CLASS>, PARENT>(MODULE, NAME) \
        .def(py::init<__VA_ARGS__>());

PYBIND11_MODULE(ir, m) {
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

    py::class_<DataType, shared_ptr<DataType>>(m, "DataType")
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
        .def(py::init<DataType, Iter>())
        .def_readonly("dtype", &Type::dtype);

    /* ExprNode and Derived Structures Declarations
        Note: ExprNode and ValNode are both pure virtual classes
    */
    py::class_<ExprNode, Expr>(m, "expr")
        .def_readonly("type", &ExprNode::type);
    py::class_<ValNode, Val, ExprNode>(m, "val");

    /* Symbol Definition */
    py::class_<Symbol, Sym, ExprNode>(m, "sym")
        .def(py::init<string, Type>())
        .def(py::init<string, Expr>());

    /* Element/Substream/Windowing and Related Type Bindings */
    py::class_<Point, shared_ptr<Point>>(m, "point")
        .def(py::init<int64_t>());
    py::class_<Window, shared_ptr<Window>>(m, "window")
        .def(py::init<int64_t, int64_t>());
    REGISTER_CLASS(Element, ValNode, m, "elem", Sym, Point)
    py::class_<LStream, shared_ptr<LStream>, ExprNode>(m, "lstream");
    REGISTER_CLASS(SubLStream, LStream, m, "sublstream", Sym, Window)

    /* Constant Expressions */
    REGISTER_CLASS(ConstNode, ValNode, m, "const", BaseType, double)

    /* Math Operators for Nary Expressions */
    py::enum_<MathOp>(m, "MathOp")
        .value("add", MathOp::ADD)
        .value("sub", MathOp::SUB)
        .value("mul", MathOp::MUL)
        .value("div", MathOp::DIV)
        .value("max", MathOp::MAX)
        .value("min", MathOp::MIN)
        .value("mod", MathOp::MOD)
        .value("sqrt", MathOp::SQRT)
        .value("pow", MathOp::POW)
        .value("abs", MathOp::ABS)
        .value("neg", MathOp::NEG)
        .value("ceil", MathOp::CEIL)
        .value("floor", MathOp::FLOOR)
        .value("lt", MathOp::LT)
        .value("lte", MathOp::LTE)
        .value("gt", MathOp::GT)
        .value("gte", MathOp::GTE)
        .value("eq", MathOp::EQ)
        .value("_not", MathOp::NOT)
        .value("_and", MathOp::AND)
        .value("_or", MathOp::OR);

    /* Nary Expressions */
    REGISTER_CLASS(NaryExpr, ValNode, m, "nary_expr", DataType, MathOp, vector<Expr>)
    REGISTER_CLASS(UnaryExpr, NaryExpr, m, "unary_expr", DataType, MathOp, Expr)
    REGISTER_CLASS(BinaryExpr, NaryExpr, m, "binary_expr", DataType, MathOp, Expr, Expr)

    /* Logical Expressions */
    REGISTER_CLASS(Exists, ValNode, m, "exists", Sym)

    /* Misc Expressions */
    REGISTER_CLASS(Get, ValNode, m, "get", Expr, size_t)
    REGISTER_CLASS(New, ValNode, m, "new", vector<Expr>)
    REGISTER_CLASS(IfElse, ExprNode, m, "ifelse", Expr, Expr, Expr)
    REGISTER_CLASS(Cast, ValNode, m, "cast", DataType, Expr)

    /* Reduction Node */
    REGISTER_CLASS(Reduce, ValNode, m, "reduce", Sym, Val, AccTy)

    /* Operator Definition */
    py::class_<OpNode, Op>(m, "op")
        .def(py::init<Iter, Params, SymTable, Expr, Sym, Aux>(),
              py::arg("iter"),
              py::arg("inputs"),
              py::arg("syms"),
              py::arg("pred"),
              py::arg("output"),
              py::arg("aux") = map<Sym, Sym>{});
}
