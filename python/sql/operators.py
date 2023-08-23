import tilt
import traceback

""" Class used to help build a TiLT Operator
"""
class TiltOpBuilder :
    def __init__(self, iter, inputs = [], syms = {}, pred = None, output = None, aux = {}) :
        self.iter = iter
        self.inputs = inputs
        self.syms = syms
        self.pred = pred
        self.output = output
        self.aux = aux

    def to_tilt_op(self) :
        return tilt.op(
            self.iter,
            self.inputs,
            self.syms,
            self.pred,
            self.output,
            self.aux
        )


""" Base Operator Class
    * Defines (pure virtual) common methods needed for all operators
"""

class QuiltOp :
    def __init__(self, name) :
        self.name = name

    def is_source(self) :
        return False

    def is_window(self) :
        return False

    def is_reduce(self) :
        return False

    def get_loop_fn(self) :
        raise NotImplementedError

    def modify_op_builders(self, op_builder) :
        raise NotImplementedError


""" Concrete Operator Definitions
    * Source
    * Map
    * Where
    * Window
    * Reduce
"""

class QuiltSource(QuiltOp) :
    def __init__(self, name : str, type : tilt.Type, batch_time : int) :
        super().__init__(name)
        self.type = type
        self.batch_time = batch_time
        # create symbols for input stream and window that wraps the entire input stream
        # so that operators which draw from this source use the same symbols in TiLT query
        self.in_stream_sym = tilt.sym(self.name, self.type)
        self.win = tilt.sublstream(self.in_stream_sym, tilt.window(-batch_time, 0))
        self.win_sym = tilt.sym("win_" + str(-batch_time) + "_" + name, self.win)

    @classmethod
    def from_datatype(cls, name : str, dt : tilt.DataType, batch_time : int) :
        return cls(name, tilt.Type(dt, tilt.Iter(0, -1)), batch_time)

    def is_source(self) :
        return True

    def modify_op_builders(self, op_builders : list[TiltOpBuilder]) :
        # checks
        assert(len(op_builders) == 0)

        # make modifications to op_builder:
        #   add input symbol
        #   set output symbol to the wrapping window
        #   set predicate to true
        op_builder = TiltOpBuilder(iter = tilt.Iter(0, self.batch_time),
                                   inputs = [self.in_stream_sym],
                                   syms = {self.win_sym : self.win},
                                   pred = tilt.const(tilt.BaseType.bool, True),
                                   output = self.win_sym)

        return op_builder


class QuiltMap(QuiltOp) :
    def __init__(self, name, map_fn) :
        super().__init__(name)
        self.map_fn = map_fn

    def loop_fn(self, in_sym) :
        e = tilt.elem(in_sym, tilt.point(0))
        e_sym = tilt.sym("e_" + in_sym.name, e)

        res = self.map_fn(e_sym)
        res_sym = tilt.sym("map_" + e_sym.name, res)

        res_op = tilt.op(
            tilt.Iter(0, 1),
            [in_sym],
            {e_sym: e, res_sym : res},
            tilt.exists(e_sym),
            res_sym
        )
        return res_op

    def modify_op_builders(self, op_builders : list[TiltOpBuilder]) :
        assert(len(op_builders) == 1)
        op_builder = op_builders[0]

        # check that op_builder.output is a pre-existing symbol
        if (op_builder.output is None) :
            raise RuntimeError("Map operator defined without a valid input.")

        map_loop = self.loop_fn(op_builder.output)
        map_loop_sym = tilt.sym(self.name, map_loop)

        op_builder.output = map_loop_sym
        op_builder.syms[map_loop_sym] = map_loop

        return op_builder


class QuiltWhere(QuiltOp) :
    def __init__(self, name, where_fn) :
        super().__init__(name)
        self.where_fn = where_fn

    def loop_fn(self, in_sym) :
        e = tilt.elem(in_sym, tilt.point(0))
        e_sym = tilt.sym("e_" + in_sym.name, e)

        pred = self.where_fn(e_sym)
        cond = tilt.binary_expr(tilt.DataType(tilt.BaseType.bool),
                                tilt.MathOp._and,
                                tilt.exists(e_sym),
                                pred)

        res_op = tilt.op(
            tilt.Iter(0, 1),
            [in_sym],
            {e_sym: e},
            cond,
            e_sym
        )
        return res_op

    def modify_op_builders(self, op_builders : list[TiltOpBuilder]) :
        assert(len(op_builders) == 1)
        op_builder = op_builders[0]

        # check that op_builder.output is a pre-existing symbol
        if (op_builder.output is None) :
            raise RuntimeError("Where operator defined without a valid input.")

        where_loop = self.loop_fn(op_builder.output)
        where_loop_sym = tilt.sym(self.name, where_loop)

        op_builder.output = where_loop_sym
        op_builder.syms[where_loop_sym] = where_loop

        return op_builder


class QuiltWindow(QuiltOp) :
    def __init__(self, name, size, stride) :
        super().__init__(name)
        assert size == stride # temporary
        self.size = size
        self.stride = stride

    def is_window(self) :
        return True

    def get_stride(self) :
        return self.stride

    def get_size(self) :
        return self.size

    def modify_op_builders(self, op_builders : list[TiltOpBuilder]) :
        assert(len(op_builders) == 1)
        return op_builders[0]


class QuiltReduce(QuiltOp) :
    def __init__(self, name, init, acc_fn, size, stride) :
        super().__init__(name)
        self.init = init
        self.acc_fn = acc_fn
        self.size = size
        self.stride = stride

    def is_reduce(self) :
        return True

    def loop_fn(self, in_sym) :
        win = tilt.sublstream(in_sym, tilt.window(-self.size, 0))
        win_sym = tilt.sym("win_" + str(self.size) + "_" + str(self.stride) + "_" + in_sym.name, win)

        red = tilt.reduce(win_sym, self.init, self.acc_fn)
        red_sym = tilt.sym("red_" + win_sym.name, red)

        red_op = tilt.op(
            tilt.Iter(0, self.stride),
            [in_sym],
            {win_sym: win, red_sym : red},
            tilt.const(tilt.BaseType.bool, True),
            red_sym
        )
        return red_op

    def modify_op_builders(self, op_builders : list[TiltOpBuilder]) :
        assert(len(op_builders) == 1)
        op_builder = op_builders[0]

        # check that op_builder.output is a pre-existing symbol
        if (op_builder.output is None) :
            raise RuntimeError("Map operator defined without a valid input.")

        red_loop = self.loop_fn(op_builder.output)
        red_loop_sym = tilt.sym(self.name, red_loop)

        op_builder.output = red_loop_sym
        op_builder.syms[red_loop_sym] = red_loop

        return op_builder

class QuiltTJoin(QuiltOp) :
    def __init__(self, name, join_fn) :
        super().__init__(name)
        self.join_fn = join_fn

    def loop_fn(self, left_sym, right_sym) :
        raise NotImplementedError

    def modify_op_builders(self, op_builders : list[TiltOpBuilder]) :
        assert(len(op_builders) == 2)
        left_op_builder = op_builders[0]
        right_op_builder = op_builders[1]

        # check that both left and right inputs have pre-existing symbols
        if ((left_op_builder.output is None) or (right_op_builder.output is None)) :
            raise RuntimeError("Join operator defined without valid inputs.")

        # create symbol for join
        join_loop = self.loop_fn(left_op_builder.output, right_op_builder.output)
        join_loop_sym = tilt.sym(self.name, join_loop)

        left_op_builder.output = join_loop_sym
        left_op_builder.syms[join_loop_sym] = join_loop

        ###  combine other components right_op_builder into left_op_builder ###
        # combine inputs
        for input in right_op_builder.inputs :
            if input not in left_op_builder.inputs :
                left_op_builder.inputs.append(input)

        # combine symbol tables
        for sym in right_op_builder.syms :
            if sym not in left_op_builder.syms :
                left_op_builder.syms[sym] = right_op_builder.syms[sym]

        # combine aux tables
        for sym in right_op_builder.aux :
            if sym not in left_op_builder.aux :
                left_op_builder.aux[sym] = right_op_builder.aux[sym]

        return left_op_builder


class QuiltTInnerJoin(QuiltTJoin) :
    def __init__(self, name, join_fn) :
        super().__init__(name, join_fn)

    def loop_fn(self, left_sym, right_sym) :
        e_left = tilt.elem(left_sym, tilt.point(0))
        e_left_sym = tilt.sym("e_left_" + left_sym.name, e_left)

        e_right = tilt.elem(right_sym, tilt.point(0))
        e_right_sym = tilt.sym("e_right_" + right_sym.name, e_right)

        pred = tilt.binary_expr(tilt.DataType(tilt.BaseType.bool),
                                tilt.MathOp._and,
                                tilt.exists(e_left_sym),
                                tilt.exists(e_right_sym))

        res = self.join_fn(e_left_sym, e_right_sym)
        res_sym = tilt.sym("join_" + e_left_sym.name + "_" + e_right_sym.name, res)

        join_op = tilt.op(
            tilt.Iter(0, 1),
            [left_sym, right_sym],
            {e_left_sym : e_left, e_right_sym : e_right, res_sym : res},
            pred,
            res_sym
        )
        return join_op

class QuiltTLeftOuterJoin(QuiltTJoin) :
    def __init__(self, name, join_fn, r_default) :
        super().__init__(name, join_fn)
        self.r_default = r_default

    def loop_fn(self, left_sym, right_sym) :
        e_left = tilt.elem(left_sym, tilt.point(0))
        e_left_sym = tilt.sym("e_left_" + left_sym.name, e_left)

        e_right = tilt.elem(right_sym, tilt.point(0))
        e_right_sym = tilt.sym("e_right_" + right_sym.name, e_right)
        e_right_val = tilt.ifelse(tilt.exists(e_right_sym),
                                  e_right_sym,
                                  self.r_default)
        e_right_val_sym = tilt.sym("e_right_" + right_sym.name + "_val", e_right_val)

        pred = tilt.exists(e_left_sym)

        res = self.join_fn(e_left_sym, e_right_val_sym)
        res_sym = tilt.sym("join_" + e_left_sym.name + "_" + e_right_val_sym.name, res)

        join_op = tilt.op(
            tilt.Iter(0, 1),
            [left_sym, right_sym],
            {e_left_sym : e_left, e_right_sym : e_right,
             e_right_val_sym : e_right_val, res_sym : res},
            pred,
            res_sym
        )
        return join_op

class QuiltTOuterJoin(QuiltTJoin) :
    def __init__(self, name, join_fn, l_default, r_default) :
        super().__init__(name, join_fn)
        self.l_default = l_default
        self.r_default = r_default

    def loop_fn(self, left_sym, right_sym) :
        e_left = tilt.elem(left_sym, tilt.point(0))
        e_left_sym = tilt.sym("e_left_" + left_sym.name, e_left)
        e_left_val = tilt.ifelse(tilt.exists(e_left_sym),
                                 e_left_sym,
                                 self.l_default)
        e_left_val_sym = tilt.sym("e_left_" + left_sym.name + "_val", e_left_val)

        e_right = tilt.elem(right_sym, tilt.point(0))
        e_right_sym = tilt.sym("e_right_" + right_sym.name, e_right)
        e_right_val = tilt.ifelse(tilt.exists(e_right_sym),
                                  e_right_sym,
                                  self.r_default)
        e_right_val_sym = tilt.sym("e_right_" + right_sym.name + "_val", e_right_val)

        pred = tilt.binary_expr(tilt.DataType(tilt.BaseType.bool),
                                tilt.MathOp._or,
                                tilt.exists(e_left_sym),
                                tilt.exists(e_right_sym))

        res = self.join_fn(e_left_val_sym, e_right_val_sym)
        res_sym = tilt.sym("join_" + e_left_val_sym.name + "_" + e_right_val_sym.name, res)

        join_op = tilt.op(
            tilt.Iter(0, 1),
            [left_sym, right_sym],
            {e_left_sym : e_left, e_right_sym : e_right,
             e_left_val_sym : e_left_val, e_right_val_sym : e_right_val,
             res_sym : res},
            pred,
            res_sym
        )
        return join_op
