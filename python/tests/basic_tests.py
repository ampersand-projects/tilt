import unittest
import random, math
import tilt
from tilt import ir, exec

class QueryTestBase(unittest.TestCase) :
    def run_op(self, query_name, op, st, et, out_reg, in_reg, eng) :
        compiled_op = eng.compile(op, query_name)
        eng.execute(compiled_op, st, et, out_reg, in_reg)

    def op_test(self, in_dt, out_dt, query_name, op, st, et, query_fn,
                input_st, input_et, input_payload, precision = None) :
        in_st = input_st[0]
        true_out_st, true_out_et, true_out = query_fn(input_st, input_et, input_payload)

        in_reg = exec.reg(len(input_payload), in_dt, in_st)
        for i in range(len(input_payload)) :
            t = input_et[i]
            in_reg.commit_data(t)
            in_reg.write_data(input_payload[i], t, in_reg.get_end_idx())

        out_reg = exec.reg(len(true_out), out_dt, st)
        eng = exec.engine()

        self.run_op(query_name, op, st, et, out_reg, in_reg, eng)

        for i in range(len(true_out)) :
            out_st = out_reg.get_ts(i)
            out_et = out_st + out_reg.get_dur(i)
            out_payload = out_reg.get_payload(i)
            self.assertEqual(out_st, true_out_st[i])
            self.assertEqual(out_et, true_out_et[i])
            if precision is None :
                self.assertEqual(out_payload, true_out[i])
            else :
                self.assertAlmostEqual(out_payload, true_out[i], precision)

    def unary_op_test(self, in_dt, out_dt, query_name, op, st, et, query_fn,
                      length, dur, precision = None) :
        input_st = [None] * length
        input_et = [None] * length
        input_payload = [None] * length
        for i in range(length) :
            st_i = dur * i
            input_st[i] = st_i
            input_et[i] = st_i + dur
            input_payload[i] = random.randrange(1000)
        self.op_test(in_dt, out_dt, query_name, op, st, et,
                     query_fn, input_st, input_et, input_payload, precision)


class MathOpTests(QueryTestBase) :
    def _Select(self, in_sym, sel_expr) :
        e = ir.elem(in_sym, ir.point(0))
        e_sym = ir.sym("e", e)
        res = sel_expr(e_sym)
        res_sym = ir.sym("res", res)
        sel_op = ir.op(
            ir.Iter(0, 1),
            [in_sym],
            {e_sym : e, res_sym : res},
            ir.exists(e_sym),
            res_sym
        )
        return sel_op

    def select_test(self, in_dt, out_dt, query_name, sel_expr, sel_fn, precision = None) :
        length = 1000
        dur = 5

        in_sym = ir.sym("in", ir.Type(in_dt, ir.Iter(0, -1)))
        sel_op = self._Select(in_sym, sel_expr)

        def sel_query_fn(input_st, input_et, input_payload) :
            true_out_st = []
            true_out_et = []
            true_out = []
            for i in range(len(input_payload)) :
                true_out_st.append(input_st[i])
                true_out_et.append(input_et[i])
                true_out.append(sel_fn(input_payload[i]))
            return true_out_st, true_out_et, true_out

        self.unary_op_test(in_dt, out_dt, query_name, sel_op,
                           0, length * dur, sel_query_fn, length, dur, precision)

    def test_add(self) :
        self.select_test(ir.DataType(ir.BaseType.i32), ir.DataType(ir.BaseType.i32),
                         "iadd",
                         lambda s : ir.binary_expr(ir.DataType(ir.BaseType.i32),
                                                   ir.MathOp._add,
                                                   s,
                                                   ir.const(ir.BaseType.i32, 10)),
                         lambda s: s + 10)
        self.select_test(ir.DataType(ir.BaseType.f32), ir.DataType(ir.BaseType.f32),
                         "fadd",
                         lambda s : ir.binary_expr(ir.DataType(ir.BaseType.f32),
                                                   ir.MathOp._add,
                                                   s,
                                                   ir.const(ir.BaseType.f32, 5)),
                         lambda s: s + 5.0)

    def test_sub(self) :
        self.select_test(ir.DataType(ir.BaseType.i32), ir.DataType(ir.BaseType.i32),
                         "isub",
                         lambda s : ir.binary_expr(ir.DataType(ir.BaseType.i32),
                                                   ir.MathOp._sub,
                                                   s,
                                                   ir.const(ir.BaseType.i32, 10)),
                         lambda s: s - 10)
        self.select_test(ir.DataType(ir.BaseType.f32), ir.DataType(ir.BaseType.f32),
                         "fsub",
                         lambda s : ir.binary_expr(ir.DataType(ir.BaseType.f32),
                                                   ir.MathOp._sub,
                                                   s,
                                                   ir.const(ir.BaseType.f32, 15)),
                         lambda s: s - 15.0)

    def test_mul(self) :
        self.select_test(ir.DataType(ir.BaseType.i32), ir.DataType(ir.BaseType.i32),
                         "imul",
                         lambda s : ir.binary_expr(ir.DataType(ir.BaseType.i32),
                                                   ir.MathOp._mul,
                                                   s,
                                                   ir.const(ir.BaseType.i32, 10)),
                         lambda s: s * 10)
        self.select_test(ir.DataType(ir.BaseType.f32), ir.DataType(ir.BaseType.f32),
                         "fmul",
                         lambda s : ir.binary_expr(ir.DataType(ir.BaseType.f32),
                                                   ir.MathOp._mul,
                                                   s,
                                                   ir.const(ir.BaseType.f32, 10)),
                         lambda s: s * 10.0,
                         precision = 5)

    def test_div(self) :
        self.select_test(ir.DataType(ir.BaseType.i32), ir.DataType(ir.BaseType.i32),
                         "idiv",
                         lambda s : ir.binary_expr(ir.DataType(ir.BaseType.i32),
                                                   ir.MathOp._div,
                                                   s,
                                                   ir.const(ir.BaseType.i32, 10)),
                         lambda s: s // 10)
        self.select_test(ir.DataType(ir.BaseType.u32), ir.DataType(ir.BaseType.u32),
                         "udiv",
                         lambda s : ir.binary_expr(ir.DataType(ir.BaseType.u32),
                                                   ir.MathOp._div,
                                                   s,
                                                   ir.const(ir.BaseType.u32, 10)),
                         lambda s: s // 10)
        self.select_test(ir.DataType(ir.BaseType.f32), ir.DataType(ir.BaseType.f32),
                         "fdiv",
                         lambda s : ir.binary_expr(ir.DataType(ir.BaseType.f32),
                                                   ir.MathOp._div,
                                                   s,
                                                   ir.const(ir.BaseType.f32, 10)),
                         lambda s: s / 10.0,
                         precision = 5)

    def test_pow(self) :
        self.select_test(ir.DataType(ir.BaseType.f32), ir.DataType(ir.BaseType.f32),
                         "fpow",
                         lambda s : ir.binary_expr(ir.DataType(ir.BaseType.f32),
                                                   ir.MathOp._pow,
                                                   s,
                                                   ir.const(ir.BaseType.f32, 2)),
                         lambda s: s ** 2,
                         precision = 5)
        self.select_test(ir.DataType(ir.BaseType.f64), ir.DataType(ir.BaseType.f64),
                         "dpow",
                         lambda s : ir.binary_expr(ir.DataType(ir.BaseType.f64),
                                                   ir.MathOp._pow,
                                                   s,
                                                   ir.const(ir.BaseType.f64, 2)),
                         lambda s: s ** 2,
                         precision = 5)

    def test_ceil(self) :
        self.select_test(ir.DataType(ir.BaseType.f32), ir.DataType(ir.BaseType.f32),
                         "fceil",
                         lambda s : ir.unary_expr(ir.DataType(ir.BaseType.f32),
                                                  ir.MathOp._ceil,
                                                  s),
                         lambda s: math.ceil(s))
        self.select_test(ir.DataType(ir.BaseType.f64), ir.DataType(ir.BaseType.f64),
                         "dceil",
                         lambda s : ir.unary_expr(ir.DataType(ir.BaseType.f64),
                                                  ir.MathOp._ceil,
                                                  s),
                         lambda s: math.ceil(s))

    def test_floor(self) :
        self.select_test(ir.DataType(ir.BaseType.f32), ir.DataType(ir.BaseType.f32),
                         "ffloor",
                         lambda s : ir.unary_expr(ir.DataType(ir.BaseType.f32),
                                                  ir.MathOp._floor,
                                                  s),
                         lambda s: math.floor(s))
        self.select_test(ir.DataType(ir.BaseType.f64), ir.DataType(ir.BaseType.f64),
                         "dfloor",
                         lambda s : ir.unary_expr(ir.DataType(ir.BaseType.f64),
                                                  ir.MathOp._floor,
                                                  s),
                         lambda s: math.floor(s))

    def test_abs(self) :
        self.select_test(ir.DataType(ir.BaseType.f32), ir.DataType(ir.BaseType.f32),
                         "fabs",
                         lambda s : ir.unary_expr(ir.DataType(ir.BaseType.f32),
                                                  ir.MathOp._abs,
                                                  s),
                         lambda s: abs(s))
        self.select_test(ir.DataType(ir.BaseType.f64), ir.DataType(ir.BaseType.f64),
                         "dabs",
                         lambda s : ir.unary_expr(ir.DataType(ir.BaseType.f64),
                                                  ir.MathOp._abs,
                                                  s),
                         lambda s: abs(s))


class QueryTests(QueryTestBase) :
    def _MovingSum(self, in_sym, dur, w) :
        e = ir.elem(in_sym, ir.point(0))
        e_sym = ir.sym("e", e)
        p = ir.elem(in_sym, ir.point(-w))
        p_sym = ir.sym("p", p)
        out = ir.out(ir.DataType(ir.BaseType.i32))
        o = ir.elem(out, ir.point(-dur))
        o_sym = ir.sym("o", o)
        p_val = ir.ifelse(ir.exists(p_sym), p_sym, ir.const(ir.BaseType.i32, 0))
        p_val_sym = ir.sym("p_val", p_val)
        o_val = ir.ifelse(ir.exists(o_sym), o_sym, ir.const(ir.BaseType.i32, 0))
        o_val_sym = ir.sym("o_val", o_val)
        res = ir.binary_expr(ir.DataType(ir.BaseType.i32),
                             ir.MathOp._sub,
                             ir.binary_expr(
                                 ir.DataType(ir.BaseType.i32),
                                 ir.MathOp._add,
                                 e_sym,
                                 o_val_sym
                             ),
                             p_val_sym)
        res_sym = ir.sym("res", res)
        sum_op = ir.op(
            ir.Iter(0, dur),
            [in_sym],
            {e_sym : e, p_sym : p, o_sym : o, p_val_sym : p_val,
             o_val_sym : o_val, res_sym : res},
            ir.exists(e_sym),
            res_sym
        )
        return sum_op

    def test_moving_sum(self) :
        length = 30
        dur = 1
        w = 10

        in_sym = ir.sym("in", ir.Type(ir.DataType(ir.BaseType.i32), ir.Iter(0, -1)))
        mov_op = self._MovingSum(in_sym, dur, w)

        def mov_query_fn(input_st, input_et, input_payload) :
            true_out_st = [None] * len(input_payload)
            true_out_et = [None] * len(input_payload)
            true_out = [None] * len(input_payload)
            for i in range(len(input_payload)) :
                out_i = i - 1
                tail_i = i - w
                payload = input_payload[i]
                if (tail_i >= 0) :
                    payload -= input_payload[tail_i]
                if (out_i >= 0) :
                    payload += true_out[out_i]
                true_out_st[i] = input_st[i]
                true_out_et[i] = input_et[i]
                true_out[i] = payload
            return true_out_st, true_out_et, true_out

        self.unary_op_test(ir.DataType(ir.BaseType.i32), ir.DataType(ir.BaseType.i32),
                           "moving_sym", mov_op, 0, length * dur, mov_query_fn, length, dur)

    def _Average(self, win_sym) :
        def acc(s, st, et, d) :
            sum = ir.get(s, 0)
            count = ir.get(s, 1)
            return ir.new([ir.binary_expr(ir.DataType(ir.BaseType.f32),
                                          ir.MathOp._add,
                                          sum, d),
                           ir.binary_expr(ir.DataType(ir.BaseType.f32),
                                          ir.MathOp._add,
                                          count,
                                          ir.const(ir.BaseType.f32, 1))])
        return ir.reduce(win_sym,
                         ir.new([ir.const(ir.BaseType.f32, 0),
                                 ir.const(ir.BaseType.f32, 0)]),
                         acc)

    def _StdDev(self, win_sym) :
        def acc(s, st, et, d) :
            sum = ir.get(s, 0)
            count = ir.get(s, 1)
            return ir.new([ir.binary_expr(ir.DataType(ir.BaseType.f32),
                                          ir.MathOp._add,
                                          sum,
                                          ir.binary_expr(ir.DataType(ir.BaseType.f32),
                                                         ir.MathOp._mul,
                                                         d, d)),
                           ir.binary_expr(ir.DataType(ir.BaseType.f32),
                                          ir.MathOp._add,
                                          count,
                                          ir.const(ir.BaseType.f32, 1))])
        return ir.reduce(win_sym,
                         ir.new([ir.const(ir.BaseType.f32, 0),
                                 ir.const(ir.BaseType.f32, 0)]),
                         acc)

    def _SelectSub(self, in_sym, avg_sym) :
        e = ir.elem(in_sym, ir.point(0))
        e_sym = ir.sym("e", e)
        res = ir.binary_expr(ir.DataType(ir.BaseType.f32),
                             ir.MathOp._sub,
                             e_sym,
                             avg_sym)
        res_sym = ir.sym("res", res)
        sel_op = ir.op(
            ir.Iter(0, 1),
            [in_sym, avg_sym],
            {e_sym : e, res_sym : res},
            ir.exists(e_sym),
            res_sym
        )
        return sel_op

    def _SelectDiv(self, in_sym, avg_sym) :
        e = ir.elem(in_sym, ir.point(0))
        e_sym = ir.sym("e", e)
        res = ir.binary_expr(ir.DataType(ir.BaseType.f32),
                             ir.MathOp._div,
                             e_sym,
                             avg_sym)
        res_sym = ir.sym("res", res)
        sel_op = ir.op(
            ir.Iter(0, 1),
            [in_sym, avg_sym],
            {e_sym : e, res_sym : res},
            ir.exists(e_sym),
            res_sym
        )
        return sel_op

    def _Norm(self, query_name, in_sym, length) :
        inwin = ir.sublstream(in_sym, ir.window(-length, 0))
        inwin_sym = ir.sym("inwin", inwin)

        avg_state = self._Average(inwin_sym)
        avg_state_sym = ir.sym(query_name + "_avg_state", avg_state)
        avg = ir.binary_expr(ir.DataType(ir.BaseType.f32),
                             ir.MathOp._div,
                             ir.get(avg_state_sym, 0),
                             ir.get(avg_state_sym, 1))
        avg_sym = ir.sym("avg", avg)

        avg_op = self._SelectSub(inwin_sym, avg_sym)
        avg_op_sym = ir.sym(query_name + "_avg_op", avg_op)

        std_state = self._StdDev(avg_op_sym)
        std_state_sym = ir.sym(query_name + "_stddev_state", std_state)
        std = ir.unary_expr(ir.DataType(ir.BaseType.f32),
                            ir.MathOp._sqrt,
                            ir.binary_expr(ir.DataType(ir.BaseType.f32),
                                           ir.MathOp._div,
                                           ir.get(std_state_sym, 0),
                                           ir.get(std_state_sym, 1)))
        std_sym = ir.sym("std", std)

        std_op = self._SelectDiv(avg_op_sym, std_sym)
        std_op_sym = ir.sym(query_name + "_stdop", std_op)

        query_op = ir.op(
            ir.Iter(0, length),
            [in_sym],
            {inwin_sym : inwin, avg_state_sym : avg_state, avg_sym : avg,
             avg_op_sym : avg_op, std_state_sym : std_state,
             std_sym : std, std_op_sym : std_op},
            ir.const(ir.BaseType.bool, True),
            std_op_sym
        )
        return query_op
    
    def test_norm(self) :
        length = 10
        dur = 1
        w = 10

        in_sym = ir.sym("in", ir.Type(ir.DataType(ir.BaseType.f32), ir.Iter(0, -1)))
        norm_op = self._Norm("norm", in_sym, w)

        def norm_query_fn(input_st, input_et, input_payload) :
            true_out_st = [None] * len(input_payload)
            true_out_et = [None] * len(input_payload)
            true_out = [None] * len(input_payload)
            num_windows = len(input_payload) // w

            for i in range(num_windows) :
                sum, mean, variance, std_dev = 0.0, 0.0, 0.0, 0.0
                for j in range(w) :
                    sum += input_payload[i * w + j]
                mean = sum / w
                for j in range(w) :
                    variance += (input_payload[i * w + j] - mean) ** 2
                std_dev = math.sqrt(variance / w)
                for j in range(w) :
                    idx = i * w + j
                    z_score = (input_payload[idx] - mean) / std_dev
                    true_out_st[idx] = input_st[idx]
                    true_out_et[idx] = input_et[idx]
                    true_out[idx] = z_score

            return true_out_st, true_out_et, true_out

        self.unary_op_test(ir.DataType(ir.BaseType.f32), ir.DataType(ir.BaseType.f32),
                           "norm", norm_op, 0, length * dur, norm_query_fn, length, dur,
                           precision = 5)


if __name__ == '__main__':
    unittest.main()
