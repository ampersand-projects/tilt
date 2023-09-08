import unittest
import tilt
from tilt import ir, exec

class TestReadMethods(unittest.TestCase) :
    def base(self, dt, f) :
        reg = exec.reg(100, dt)
        for i in range(100) :
            reg.commit_data((i + 1) * 3)
            reg.write_data(f(i), (i + 1) * 3, reg.get_end_idx())
        for i in range(100) :
            self.assertEqual(i * 3, reg.get_ts(i))
            self.assertEqual(3, reg.get_dur(i))
            self.assertEqual(f(i), reg.get_payload(i))

    def test_int(self) :
        self.base(ir.DataType(ir.BaseType.i8), lambda x : x)
        self.base(ir.DataType(ir.BaseType.i16), lambda x : x)
        self.base(ir.DataType(ir.BaseType.i32), lambda x : x)
        self.base(ir.DataType(ir.BaseType.i64), lambda x : x)
        self.base(ir.DataType(ir.BaseType.u8), lambda x : x)
        self.base(ir.DataType(ir.BaseType.u16), lambda x : x)
        self.base(ir.DataType(ir.BaseType.u32), lambda x : x)
        self.base(ir.DataType(ir.BaseType.u64), lambda x : x)

    def test_float(self) :
        self.base(ir.DataType(ir.BaseType.f32), lambda x : x + 0.5)
        self.base(ir.DataType(ir.BaseType.f64), lambda x : x + 0.5)

    def test_struct(self) :
        self.base(ir.DataType(ir.BaseType.struct,
                              [ir.DataType(ir.BaseType.i16),
                               ir.DataType(ir.BaseType.f32)]),
                  lambda x : [x - 2, x * 2.5])

if __name__ == '__main__':
    unittest.main()
