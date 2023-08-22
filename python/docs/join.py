import tilt

env = tilt.TiLTEnv()

env.set_start_time(0)
env.set_end_time(200)

left_stream = env.create_input_from_dt(200, tilt.DataType(tilt.BaseType.f32))
for i in range(200):
    left_stream.commit_data(i + 1)
    left_stream.write_data(1, i + 1, left_stream.get_data_end_idx())

def map_fn_0(in_sym) :
    return tilt.binary_expr(tilt.DataType(tilt.BaseType.f32),
                            tilt.MathOp._add,
                            in_sym,
                            tilt.const(tilt.BaseType.f32, 3))

def map_fn_1(in_sym) :
    return tilt.binary_expr(tilt.DataType(tilt.BaseType.f32),
                            tilt.MathOp._add,
                            in_sym,
                            tilt.const(tilt.BaseType.f32, 15))

def join_fn(left_sym, right_sym) :
    return tilt.binary_expr(tilt.DataType(tilt.BaseType.f32),
                            tilt.MathOp._add,
                            left_sym,
                            right_sym)


right_stream = left_stream.map(map_fn_0)
out_stream = left_stream.map(map_fn_1).tjoin(right_stream, join_fn)
env.compile_graph(out_stream)
print("execute")
env.execute()

out_stream.print_data()
