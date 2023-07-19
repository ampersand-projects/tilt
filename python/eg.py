import tilt
from tilt import ir, utils, region

### Example 1 ###
### Select Query ###
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

float_in_reg = region.reg(100, ir.DataType(ir.BaseType.f32))
for i in range(100):
    float_in_reg.commit_data(i + 1)
    float_in_reg.write_data(i + 0.5, i + 1, float_in_reg.get_end_idx())
print(float_in_reg)
select_out_reg = region.reg(100, ir.DataType(ir.BaseType.f32))
compiled_sel = utils.compile(sel_op, "querysel")
region.execute(compiled_sel, 0, 100, select_out_reg, [float_in_reg])
print(select_out_reg)


### Example 2 ###
### Sum Query ###
acc_lambda = lambda s, st, et, d : ir.binary_expr(ir.DataType(ir.BaseType.f32),
                                                  ir.MathOp.add,
                                                  s, d)
win = ir.sublstream(in_stream, ir.window(-10, 0))
win_sym = ir.sym("win", win)
sum = ir.reduce(win_sym, ir.const(ir.BaseType.f32, 0), acc_lambda)
sum_sym = ir.sym("sum", sum)
sum_op = ir.op(
    ir.Iter(0, 10),
    [in_stream],
    {win_sym : win, sum_sym : sum},
    ir.const(ir.BaseType.bool, True),
    sum_sym
)

utils.print_IR(sum_op)
sum_out_reg = region.reg(10, ir.DataType(ir.BaseType.f32))
compiled_sum = utils.compile(sum_op, "querysum")
region.execute(compiled_sum, 0, 100, sum_out_reg, [float_in_reg])
print(sum_out_reg)

### Example 3 ###
### Populate an input stream with a structured data type ###
struct_dt = ir.DataType(
                ir.BaseType.struct,
                [ir.DataType(ir.BaseType.i16),
                 ir.DataType(ir.BaseType.f32)])
struct_reg = region.reg(4, struct_dt)
struct_reg.commit_data(1)
struct_reg.write_data([1, 32.5], 1, struct_reg.get_end_idx())
struct_reg.commit_data(3)
struct_reg.write_data([2, -2], 3, struct_reg.get_end_idx())
struct_reg.commit_data(7)
struct_reg.write_data([4, 15.9], 7, struct_reg.get_end_idx())
print(struct_reg)
