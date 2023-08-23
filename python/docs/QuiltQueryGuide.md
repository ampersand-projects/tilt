# Writing Streaming Queries Using QuiLT

In addition to the standard TiLT API, TiLT provides a SQL-like API for writing simple streaming queries for users who are more comfortable with databases or other streaming engines such as Flink and Trill.
We refer to this SQL-like API as _QuiLT_.

Below, we provide a basic example highlighting the common structure of streaming programs written with QuiLT.

```python
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
def sum_fn(s, st, et, d) :
    return tilt.binary_expr(tilt.DataType(tilt.BaseType.f32),
                            tilt.MathOp._add, s, d)

out_stream = in_stream.window(10, 10) \
                .reduce(tilt.const(tilt.BaseType.f32, 0), sum_fn) \
                .map(map_fn)

env.compile_graph(out_stream)
env.execute()

out_stream.print_data()
```

1. Create a TiLT environment.
The environment is required to create TiLT datastreams, compile query graphs, and execute streaming queries.
Users must also set start time and end time variables for execution within the environment before creating input streams.
2. Create a TiLT input stream.
When creating the input stream, the buffer size and input data schema needs to be specified.
We support arbitrary and nested structures of integer and floating point data, similar to NumPy's structured datatype.
3. Define a query graph using QuiLT operators.
4. Compile the query graph.
In this step, the environment lowers the QuiLT graph into TiLT IR, which is then compiled into an executable that the environment maintains.
5. Execute the query.

## QuiLT Operators

Several of the operators below take in a user-defined function as an argument.
These functions should be written with the TiLT API, take in TiLT expressions as input, and return TiLT expressions.

### Map

`map(map_fn)` takes a datastream as input and returns a datastream with `map_fn` applied to each element in the input stream.
A query which adds 3 to every element in the input stream can be written as follows:

```python
def map_fn(in_sym) :
    return tilt.binary_expr(tilt.DataType(tilt.BaseType.f32),
                            tilt.MathOp._add,
                            in_sym,
                            tilt.const(tilt.BaseType.f32, 3))

out_stream = in_stream.map(map_fn)
```

### Where

`where(where_fn)` takes a datastream as input and returns a datastream with only the elements in the input stream for which `where_fn` holds true.
A query which filters an input stream so that the output only contains values greater than 0 can be written as follows:

```python
def where_fn(in_sym) :
    return tilt.binary_expr(tilt.DataType(tilt.BaseType.bool),
                            tilt.MathOp._gt,
                            in_sym,
                            tilt.const(tilt.BaseType.f32, 0))

out_stream = in_stream.where(where_fn)
```

### Window

`window(size, stride)` creates a window over the input stream.
Window operators must immediately be followed by a Reduce operator in the query graph.

### Reduce

`reduce(init, acc_fn)` takes a windowed stream as input, and reduces each window into a scalar value.
The `acc_fn` should take in 4 arguments: `s` to denote state, `st` and `et` to denote start and end times respectively, and `d` to denote input payloads.
The scalar value is computed by applying the `acc_fn` to each input in the window with `init` as the initial state.

A query which computes a windowed sum can be written as follows:

```python
def sum_fn(s, st, et, d) :
    return tilt.binary_expr(tilt.DataType(tilt.BaseType.f32),
                            tilt.MathOp._add, s, d)

out_stream = in_stream.window(10, 10) \
                .reduce(tilt.const(tilt.BaseType.f32, 0), sum_fn)
```

### TInnerJoin
`tijoin(right, join_fn)` performs a temporal inner join between the current stream and the `right` stream, with the join operation determined by the user defined `join_fn`.
In particular, the `join_fn` is applied to overlapping events in both streams.

A query which adds together overlapping elements of two streams can be written as follows:

```python
def join_fn(left_sym, right_sym) :
    return tilt.binary_expr(tilt.DataType(tilt.BaseType.f32),
                            tilt.MathOp._add, left_sym, right_sym)

out_stream = left_stream.tjoin(right_stream, join_fn)
```

### TLeftOuterJoin
`tlojoin(right, join_fn, r_default)` performs a temporal left outer join between the current stream and the `right` stream, with the join operation determined by the user defined `join_fn`.
In particular, the `join_fn` is applied to overlapping event sin both streams, and `r_default` is used in place when there are no corresponding events in the `right` stream.

### TOuterJoin
`tojoin(right, join_fn, l_default, r_default)` performs a temporal full outer join between the current stream dn the `right` stream, with the join operation determined by the user defined `join_fn`.
In particular, the `join_fn` is applied to overlapping event sin both streams, and `l_default`, `r_default` are used in place when there are no corresponding events in the `left` or `right` streams respectively.

## Improving Execution

When writing QuiLT queries, we recommend that a Window operator is not preceded by a Map or Where operator at any point in the query graph.

As an example, consider a query that adds 3 to every payload in the input stream, and then computes a windowed sum over the intermediate stream.
One might be tempted to write the following query:

```python
def map_fn(in_sym) :
    return tilt.binary_expr(tilt.DataType(tilt.BaseType.f32),
                            tilt.MathOp._add,
                            in_sym,
                            tilt.const(tilt.BaseType.f32, 3))
def sum_fn(s, st, et, d) :
    return tilt.binary_expr(tilt.DataType(tilt.BaseType.f32),
                            tilt.MathOp._add, s, d)

out_stream = in_stream.map(map_fn) \
                .window(10, 10) \
                .reduce(tilt.const(tilt.BaseType.f32, 0), sum_fn)
```

This will result in an inefficient query since TiLT will apply the `map_fn` over the entire input before the Reduce computation.
Instead, the addition of 3 should be encoded in the `acc_fn` argument of the Reduce operator as follows:

```python
def acc_fn(s, st, et, d) :
    # add(s, add(d, 3))
    return tilt.binary_expr(tilt.DataType(tilt.BaseType.f32),
                            tilt.MathOp._add,
                            s,
                            tilt.binary_expr(
                                tilt.DataType(tilt.BaseType.f32),
                                tilt.MathOp._add,
                                d,
                                tilt.const(tilt.BaseType.f32, 3)
                            ))

out_stream = in_stream.window(10, 10) \
                .reduce(tilt.const(tilt.BaseType.f32, 0), acc_fn)
```

Similarly, a query which a computes a windowed sum over only values which are greater than 0 should be written as follows:

```python
def acc_fn(s, st, et, d) :
    # ifelse(d > 0, add(s, d), s)
    return tilt.ifelse(tilt.binary_expr(tilt.DataType(tilt.BaseType.bool),
                                        tilt.MathOp._gt,
                                        d,
                                        tilt.const(tilt.BaseType.f32), 0),
                       tilt.binary_expr(tilt.DataType(tilt.BaseType.f32),
                                        tilt.MathOp._add,
                                        s, d),
                       s)

out_stream = in_stream.window(10, 10) \
                .reduce(tilt.const(tilt.BaseType.f32, 0), acc_fn)
```

## In the Works

In the future, we plan to also provide support for the following operators:
1. Event time shifting
2. Multicasting (i.e. defining parallel windows over the same input stream)
