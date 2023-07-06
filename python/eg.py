import tilt
from tilt import ir, utils, region

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

utils.print_IR(sel_op)


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

utils.print_IR(sum_op)


### Example 3 ###
### Populate an input stream ###
in_reg = region.reg(4, ir.DataType(ir.BaseType.f32))
print(in_reg)
in_reg.commit_data(10)
in_reg.write_data(1.5, 10, in_reg.get_end_idx())
in_reg.commit_data(15)
in_reg.write_data(2.2, 15, in_reg.get_end_idx())
print(in_reg)


### Example 4 ###
### Populate an input stream with a simple structured data type ###
struct_dt = ir.DataType(
                ir.BaseType.struct,
                [ir.DataType(ir.BaseType.f32),
                 ir.DataType(ir.BaseType.i16)])
struct_reg = region.reg(4, struct_dt)
struct_reg.commit_data(1)
struct_reg.write_data([1.5, 32], 1, struct_reg.get_end_idx())
struct_reg.commit_data(3)
struct_reg.write_data([2, -2], 3, struct_reg.get_end_idx())
struct_reg.commit_data(7)
struct_reg.write_data([3.9, 15], 7, struct_reg.get_end_idx())
print(struct_reg)


### Example 5 ###
### Populate an input stream with a nested structured data type ###
nest_dt = ir.DataType(
    ir.BaseType.struct,
    [
        ir.DataType(
            ir.BaseType.struct,
            [ir.DataType(ir.BaseType.i64), ir.DataType(ir.BaseType.i64)]
        ),
        ir.DataType(
            ir.BaseType.struct,
            [ir.DataType(ir.BaseType.f32), ir.DataType(ir.BaseType.u16)]
        )
    ]
)
nest_reg = region.reg(5, nest_dt)
for i in range(5):
    nest_reg.commit_data(i+1)
    nest_reg.write_data([[i, -i], [i + 0.5, i + 1]],
                        i+1, nest_reg.get_end_idx())
print(nest_reg)
