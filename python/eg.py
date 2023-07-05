import tilt
from tilt import ir

### Example 1 ###
### Print out the Loop IR for a Select operator ###
in_stream = ir.sym("in",
                   ir.Type(ir.DataType(ir.BaseType.f32),
                           ir.Iter(0, -1)))
e = ir.elem(in_stream, ir.point(0))
e_sym = ir.sym("e", e)
res = ir.binary_expr(ir.DataType(ir.BaseType.f32),
                     ir.MathOp.add,
                     e_sym,
                     ir.const(ir.BaseType.f32, 3))
res_sym = ir.sym("res", res)
sel_op = ir.op(
    ir.Iter(0, 1),
    [in_stream],
    {e_sym : e, res_sym : res},
    ir.exists(e_sym),
    res_sym
)

ir.print_IR(sel_op)


### Example 2 ###
### Print out the Loop IR for a Sum operator ###
acc_lambda = lambda s, st, et, d : ir.binary_expr(ir.DataType(ir.BaseType.f32),
                                                  ir.MathOp.add,
                                                  s, d)
win = ir.sublstream(in_stream, ir.window(-100, 0))
win_sym = ir.sym("win", win)
sum = ir.reduce(win_sym, ir.const(ir.BaseType.f32, 0), acc_lambda)
sum_sym = ir.sym("sum", sum)
sum_op = ir.op(
    ir.Iter(0, 100),
    [in_stream],
    {win_sym : win, sum_sym : sum},
    ir.const(ir.BaseType.bool, True),
    sum_sym
)

ir.print_IR(sum_op)
