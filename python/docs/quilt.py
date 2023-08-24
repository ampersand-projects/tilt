import tilt

env = tilt.TiLTEnv()
env.set_start_time(0)
env.set_end_time(200)

in_stream = env.create_input_from_dt(200, tilt.DataType(tilt.BaseType.f32))
for i in range(200):
    in_stream.commit_data(i + 1)
    in_stream.write_data(1, i + 1, in_stream.get_data_end_idx())
in_stream.print_data()

def map_fn(in_sym) :
    return tilt.binary_expr(tilt.DataType(tilt.BaseType.f32),
                            tilt.MathOp._add,
                            in_sym,
                            tilt.const(tilt.BaseType.f32, 3))

def where_fn(in_sym) :
    return tilt.binary_expr(tilt.DataType(tilt.BaseType.bool),
                            tilt.MathOp._gt,
                            in_sym,
                            tilt.const(tilt.BaseType.f32, 0))

def sum_fn(s, st, et, d) :
    return tilt.binary_expr(tilt.DataType(tilt.BaseType.f32),
                            tilt.MathOp._add, s, d)

def mul_fn(s, st, et, d) :
    return tilt.binary_expr(tilt.DataType(tilt.BaseType.f32),
                            tilt.MathOp._mul, s, d)

# out_stream = in_stream.map(map_fn)
# out_stream = in_stream.window(10, 10).reduce(tilt.const(tilt.BaseType.f32, 0), sum_fn)
out_stream = in_stream.shift(5) \
                .window(10, 10) \
                .reduce(tilt.const(tilt.BaseType.f32, 0), sum_fn)

"""
out_stream = in_stream.window(10, 10) \
                .reduce(tilt.const(tilt.BaseType.f32, 0), sum_fn) \
                .map(map_fn)
"""

"""
out_stream = in_stream.map(map_fn) \
                .window(10, 10) \
                .reduce(tilt.const(tilt.BaseType.f32, 0), sum_fn)
"""

"""
out_stream = in_stream.window(10, 10) \
                .reduce(tilt.const(tilt.BaseType.f32, 0), sum_fn) \
                .window(100, 100) \
                .reduce(tilt.const(tilt.BaseType.f32, 1), mul_fn)
"""

env.compile_graph(out_stream)
env.execute()

out_stream.print_data()
