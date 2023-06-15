#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include <vector>
#include <memory>
#include <string>
#include <fstream>

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

void print_loopIR( Op query_op, string fname )
{
    auto query_op_sym = _sym("query", query_op);
    auto loop = LoopGen::Build(query_op_sym, query_op.get());

    ofstream f;
    f.open(fname);
    f << IRPrinter::Build(loop);
    f.close();
}

#define REGISTER_NOINIT_CLASS(CLASS, PARENT, MODULE, NAME) \
    py::class_<CLASS, shared_ptr<CLASS>, PARENT>( MODULE, NAME );

#define REGISTER_INIT_CLASS(CLASS, PARENT, MODULE, NAME, ...) \
    py::class_<CLASS, shared_ptr<CLASS>, PARENT>( MODULE, NAME ) \
        .def( py::init<__VA_ARGS__>() );

PYBIND11_DECLARE_HOLDER_TYPE( ConstNode, _expr<ConstNode> );

PYBIND11_MODULE( pytilt, m ) {

    /* Structures related to TiLT typing */
    py::enum_<BaseType>( m, "BaseType" )
        .value( "bool", BaseType::BOOL )
        .value( "i8", BaseType::INT8 )
        .value( "i16", BaseType::INT16 )
        .value( "i32", BaseType::INT32 )
        .value( "i64", BaseType::INT64 )
        .value( "u8", BaseType::UINT8 )
        .value( "u16", BaseType::UINT16 )
        .value( "u32", BaseType::UINT32 )
        .value( "u64", BaseType::UINT64 )
        .value( "f32", BaseType::FLOAT32 )
        .value( "f64", BaseType::FLOAT64 )
        .value( "struct", BaseType::STRUCT )
        .value( "ptr", BaseType::PTR )
        .value( "t", BaseType::TIME )
        .value( "idx", BaseType::INDEX )
        .value( "ival", BaseType::IVAL );

    py::class_<DataType>( m, "DataType" )
        .def( py::init<BaseType, vector<DataType>, size_t>(),
              py::arg( "btype" ),
              py::arg( "dtypes" ) = vector<DataType>{},
              py::arg( "size" ) = 0 )
        .def( "str", &DataType::str );

    py::class_<Iter>( m, "Iter" )
        .def( py::init<int64_t, int64_t>(),
              py::arg( "offset" ) = 0,
              py::arg( "period" ) = 0 )
        .def( "str", &Iter::str );

    py::class_<Type>( m, "Type" )
        .def( py::init<DataType, Iter>() );

    /* ExprNode and Derived Structures Declarations */
    py::class_<ExprNode, Expr>( m, "expr" )
        .def( "getType",
              []( Expr expr ) {
                    return expr->type;
              } );
    REGISTER_NOINIT_CLASS( ValNode, ExprNode, m, "val" )

    /* Element/Substream/Windowing and Related Type Bindings */
    REGISTER_NOINIT_CLASS( Element, ValNode, m, "elem" )
    REGISTER_NOINIT_CLASS( LStream, ExprNode, m, "lstream" )
    REGISTER_NOINIT_CLASS( SubLStream, LStream, m, "sublstream" )

    /* Symbol Definition */
    py::class_<Symbol, Sym, ExprNode>( m, "sym" )
        .def( py::init<string, Type>() )
        .def( "point",
              []( Sym sym, int64_t o ) {
                    return _expr_elem( sym, Point(o) );
              } )
        .def( "window",
              []( Sym sym, int64_t start, int64_t end ) {
                    return _expr_subls( sym, Window( start, end ) );
              } )
        .def( "getType",
              []( Sym sym ) {
                    return sym->type;
              } );

    /* Constant Expressions */
    REGISTER_NOINIT_CLASS( ConstNode, ValNode, m, "const" )
    m.def( "i8", &_i8 );
    m.def( "i16", &_i16 );
    m.def( "i32", &_i32 );
    m.def( "i64", &_i64 );
    m.def( "u8", &_u8 );
    m.def( "u16", &_u16 );
    m.def( "u32", &_u32 );
    m.def( "u64", &_u64 );
    m.def( "f32", &_f32 );
    m.def( "f64", &_f64 );
    // m.def( "ch", &_ch ); /* tilt::tilder::_ch is declared but not defined */
    m.def( "ts", &tilt::tilder::_ts ); /* _ts is also a struct defined in cpython */
    m.def( "idx", &_idx );
    m.def( "true", &_true );
    m.def( "false", &_false );

    /* Nary Expressions */
    REGISTER_NOINIT_CLASS( NaryExpr, ValNode, m, "nary_expr" )
    REGISTER_NOINIT_CLASS( UnaryExpr, NaryExpr, m, "unary_expr" )
    REGISTER_NOINIT_CLASS( BinaryExpr, NaryExpr, m, "binary_expr" )

#define REGISTER_UNARY_EXPR( EXPR, NAME ) \
    py::class_<EXPR, shared_ptr<EXPR>, UnaryExpr>( m, NAME ) \
        .def( py::init<Expr>() );

#define REGISTER_BINARY_EXPR(EXPR, NAME) \
    py::class_<EXPR, shared_ptr<EXPR>, BinaryExpr>( m, NAME ) \
        .def( py::init<Expr, Expr>() );

    /* Arithmetic Expressions */
    REGISTER_BINARY_EXPR( Add, "add" )
    REGISTER_BINARY_EXPR( Sub, "sub" )
    REGISTER_BINARY_EXPR( Mul, "mul" )
    REGISTER_BINARY_EXPR( Div, "div" )
    REGISTER_BINARY_EXPR( Max, "max" )
    REGISTER_BINARY_EXPR( Min, "min" )
    REGISTER_UNARY_EXPR( Abs, "abs" )
    REGISTER_UNARY_EXPR( Neg, "neg" )
    REGISTER_BINARY_EXPR( Mod, "mod" )
    REGISTER_UNARY_EXPR( Sqrt, "sqrt" )
    REGISTER_BINARY_EXPR( Pow, "pow" )
    REGISTER_UNARY_EXPR( Ceil, "ceil" )
    REGISTER_UNARY_EXPR( Floor, "floor" )
    REGISTER_BINARY_EXPR( LessThan, "lt" )
    REGISTER_BINARY_EXPR( LessThanEqual, "lte" )
    REGISTER_BINARY_EXPR( GreaterThan, "gt" )
    REGISTER_BINARY_EXPR( GreaterThanEqual, "gte" )
    REGISTER_BINARY_EXPR( Equals, "eq" )

    /* Logical Expressions */
    REGISTER_INIT_CLASS( Exists, ValNode, m, "exists", Sym )
    REGISTER_UNARY_EXPR( Not, "not" )
    REGISTER_BINARY_EXPR( And, "and" )
    REGISTER_BINARY_EXPR( Or, "or" )

    /* Misc Expressions */
    REGISTER_INIT_CLASS( Get, ValNode, m, "get", Expr, size_t )
    REGISTER_INIT_CLASS( New, ValNode, m, "new", vector<Expr> )
    REGISTER_INIT_CLASS( IfElse, ExprNode, m, "ifelse", Expr, Expr, Expr )
    REGISTER_INIT_CLASS( Cast, ValNode, m, "cast", DataType, Expr )

    /* Reduction Node */
    REGISTER_INIT_CLASS( Reduce, ValNode, m, "reduce", Sym, Val, AccTy )

#undef REGISTER_UNARY_EXPR
#undef REGISTER_BINARY_EXPR

    /* Operator Definition */
    py::class_<OpNode, Op>( m, "op" )
        .def( py::init<Iter, Params, SymTable, Expr, Sym, Aux>(),
              py::arg( "iter" ),
              py::arg( "inputs" ),
              py::arg( "syms" ),
              py::arg( "pred" ),
              py::arg( "output" ),
              py::arg( "aux" ) = map<Sym, Sym>{} );

    /* Temp */
    m.def( "print_loopIR", &print_loopIR );
}