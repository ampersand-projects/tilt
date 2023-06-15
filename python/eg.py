import pytilt as pt

### Example 1 ###
### Print out the Loop IR for a Select operator into select_loopIR.txt ###
in_stream = pt.sym( "in", pt.Type( pt.DataType( pt.BaseType.f32, [] ), pt.Iter(0, -1)) )
e = in_stream.point( 0 )
e_sym = pt.sym( "e", e.getType() )
res = pt.add( e_sym, pt.f32(3) )
res_sym = pt.sym( "res", res.getType() )
sel_op = pt.op(
    pt.Iter(0, 1),
    [in_stream],
    { e_sym : e, res_sym : res },
    pt.exists(e_sym),
    res_sym
)

pt.print_loopIR(sel_op, "select_loopIR.txt")


### Example 2 ###
### Print out the Loop IR for a Sum operator into sum_loopIR.txt ###
def acc( s : pt.expr, st : pt.expr, et : pt.expr, d : pt.expr ) -> pt.expr :
    return pt.add(s, d)
acc_lambda = lambda s, st, et, d : pt.add(s, d)
win = in_stream.window(-100, 0)
win_sym = pt.sym( "win", win.getType() )
sum = pt.reduce( win_sym, pt.f32(0), acc_lambda )
sum_sym = pt.sym( "sum", sum.getType() )
sum_op = pt.op(
    pt.Iter(0, 100),
    [in_stream],
    {win_sym : win, sum_sym : sum},
    pt.true(),
    sum_sym
)

pt.print_loopIR(sum_op, "sum_loopIR.txt")