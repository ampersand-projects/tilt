import tilt
from tilt import ir, utils, exec

tilt_eng = exec.engine()

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

float_in_reg = exec.reg(100, ir.DataType(ir.BaseType.f32))
for i in range(100):
    float_in_reg.commit_data(i + 1)
    float_in_reg.write_data(i + 0.5, i + 1, float_in_reg.get_end_idx())
print(float_in_reg)
select_out_reg = exec.reg(100, ir.DataType(ir.BaseType.f32))
compiled_sel = tilt_eng.compile(sel_op, "querysel")
tilt_eng.execute(compiled_sel, 0, 100, select_out_reg, [float_in_reg])
print(select_out_reg)
print(float_in_reg)

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
sum_out_reg = exec.reg(10, ir.DataType(ir.BaseType.f32))
compiled_sum = tilt_eng.compile(sum_op, "query_sum")
tilt_eng.execute(compiled_sum, 0, 100, sum_out_reg, [float_in_reg])
print(sum_out_reg)


### Example 3 ###
### Structured data type ###
struct_dt = ir.DataType(
                ir.BaseType.struct,
                [ir.DataType(ir.BaseType.i8),
                 ir.DataType(ir.BaseType.f32)])
struct_reg = exec.reg(10, struct_dt)
for i in range(10):
    struct_reg.commit_data(i + 1)
    struct_reg.write_data([i, i + 0.5], i + 1, struct_reg.get_end_idx())
print(struct_reg)

struct_stream = ir.sym("struct_in", ir.Type(struct_dt, ir.Iter(0, -1)))
struct_e = ir.elem(struct_stream, ir.point(0))
struct_e_sym = ir.sym("struct_e", struct_e)
struct_res = ir.new([ir.get(struct_e_sym, 0),
                     ir.binary_expr(
                         ir.DataType(ir.BaseType.f32),
                         ir.MathOp.mul,
                         ir.get(struct_e_sym, 1),
                         ir.const(ir.BaseType.f32, 3)
                     )])
struct_res_sym = ir.sym("struct_res", struct_res)
struct_op = ir.op(
    ir.Iter(0, 1),
    [struct_stream],
    {struct_e_sym : struct_e, struct_res_sym : struct_res},
    ir.exists(struct_e_sym),
    struct_res_sym
)

utils.print_IR(struct_op)
struct_out_reg = exec.reg(10, struct_dt)
compiled_struct = tilt_eng.compile(struct_op, "query_struct")
tilt_eng.execute(compiled_struct, 0, 10, struct_out_reg, [struct_reg])
print(struct_out_reg)


### Example 4 ###
### Join Query ###
right_reg = exec.reg(100, ir.DataType(ir.BaseType.f32))
for i in range(100):
    right_reg.commit_data(i + 1)
    right_reg.write_data(i * 0.5, i + 1, right_reg.get_end_idx())

right_stream = ir.sym("right",
                      ir.Type(ir.DataType(ir.BaseType.f32),
                              ir.Iter(0, -1)))
e_right = ir.elem(right_stream, ir.point(0))
e_right_sym = ir.sym("e_right", e_right)
join_res = ir.binary_expr(ir.DataType(ir.BaseType.f32),
                          ir.MathOp.add,
                          e_sym,
                          e_right_sym)
join_res_sym = ir.sym("join_res", join_res)
join_pred = ir.binary_expr(ir.DataType(ir.BaseType.bool),
                           ir.MathOp._and,
                           ir.exists(e_sym),
                           ir.exists(e_right_sym))
join_op = ir.op(
    ir.Iter(0, 1),
    [in_stream, right_stream],
    {e_sym : e, e_right_sym : e_right,
     join_res_sym : join_res},
    join_pred,
    join_res_sym
)

utils.print_IR(join_op)
join_out_reg = exec.reg(100, ir.DataType(ir.BaseType.f32))
compiled_join = tilt_eng.compile(join_op, "query_join")
tilt_eng.execute(compiled_join, 0, 100, join_out_reg, [float_in_reg, right_reg])
print(join_out_reg)


### Example 5 ###
### Nested structured data type ###
nest_dt = ir.DataType(
    ir.BaseType.struct,
    [
        ir.DataType(
            ir.BaseType.struct,
            [ir.DataType(ir.BaseType.i64), ir.DataType(ir.BaseType.i64)]
        ),
        ir.DataType(ir.BaseType.i8),
        ir.DataType(
            ir.BaseType.struct,
            [ir.DataType(ir.BaseType.f32), ir.DataType(ir.BaseType.u16)]
        )
    ]
)
nest_reg = exec.reg(10, nest_dt)
for i in range(10):
    nest_reg.commit_data(i + 1)
    nest_reg.write_data([[i, -i], i - 1, [i + 0.5, i + 1]],
                        i + 1, nest_reg.get_end_idx())
print(nest_reg)

nest_stream = ir.sym("nest_in", ir.Type(nest_dt, ir.Iter(0, -1)))
nest_e = ir.elem(nest_stream, ir.point(0))
nest_e_sym = ir.sym("nest_e", nest_e)
nest_res = ir.new([ir.new(
                    [ir.const(ir.BaseType.i64, 15),
                     ir.binary_expr(
                        ir.DataType(ir.BaseType.i64),
                        ir.MathOp.mul,
                        ir.get(ir.get(nest_e_sym, 0), 1),
                        ir.const(ir.BaseType.i64, -2)
                     )]),
                   ir.binary_expr(
                        ir.DataType(ir.BaseType.i8),
                        ir.MathOp.sub,
                        ir.get(nest_e_sym, 1),
                        ir.const(ir.BaseType.i8, 2)
                    ),
                   ir.get(nest_e_sym, 2)
                  ])
nest_res_sym = ir.sym("nest_res", nest_res)
nest_op = ir.op(
    ir.Iter(0, 1),
    [nest_stream],
    {nest_e_sym : nest_e, nest_res_sym : nest_res},
    ir.exists(nest_e_sym),
    nest_res_sym
)

utils.print_IR(nest_op)
nest_out_reg = exec.reg(10, nest_dt)
compiled_nest = tilt_eng.compile(nest_op, "query_nest")
tilt_eng.execute(compiled_nest, 0, 10, nest_out_reg, [nest_reg])
print(nest_out_reg)
