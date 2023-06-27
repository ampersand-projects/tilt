import pytilt as pt

### Example 1 ###
### Print out the Loop IR for a Select operator ###
in_stream = pt.sym("in",
                   pt.Type(pt.DataType(pt.BaseType.f32),
                           pt.Iter(0, -1)))
e = pt.elem(in_stream, pt.point(0))
e_sym = pt.sym("e", e)
res = pt.binary_expr(pt.DataType(pt.BaseType.f32),
                     pt.MathOp.add,
                     e_sym,
                     pt.const(pt.BaseType.f32, 3))
res_sym = pt.sym("res", res)
sel_op = pt.op(
    pt.Iter(0, 1),
    [in_stream],
    {e_sym : e, res_sym : res},
    pt.exists(e_sym),
    res_sym
)

pt.print_IR(sel_op)


### Example 2 ###
### Print out the Loop IR for a Sum operator ###
acc_lambda = lambda s, st, et, d : pt.binary_expr(pt.DataType(pt.BaseType.f32),
                                                  pt.MathOp.add,
                                                  s, d)
win = pt.sublstream(in_stream, pt.window(-100, 0))
win_sym = pt.sym("win", win)
sum = pt.reduce(win_sym, pt.const(pt.BaseType.f32, 0), acc_lambda)
sum_sym = pt.sym("sum", sum)
sum_op = pt.op(
    pt.Iter(0, 100),
    [in_stream],
    {win_sym : win, sum_sym : sum},
    pt.const(pt.BaseType.bool, True),
    sum_sym
)

pt.print_IR(sum_op)
