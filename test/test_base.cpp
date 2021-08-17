#include <utility>
#include <cmath>
#include <algorithm>

#include "test_base.h"
#include "test_query.h"

#include "tilt/codegen/loopgen.h"
#include "tilt/codegen/llvmgen.h"
#include "tilt/engine/engine.h"
#include "tilt/codegen/vinstr.h"

using namespace tilt;
using namespace tilt::tilder;

void run_op(Op op, ts_t st, ts_t et, region_t* out_reg, region_t* in_reg, string query_name)
{
    auto op_sym = op->sym(query_name);
    auto loop = LoopGen::Build(op_sym, op.get());

    auto jit = ExecEngine::Get();
    auto& llctx = jit->GetCtx();

    auto llmod = LLVMGen::Build(loop, llctx);
    jit->AddModule(move(llmod));

    auto loop_addr = (region_t* (*)(ts_t, ts_t, region_t*, region_t*)) jit->Lookup(loop->get_name());

    loop_addr(st, et, out_reg, in_reg);
}

template<typename InTy, typename OutTy>
void op_test(Op op, QueryFn<InTy, OutTy> query_fn, vector<Event<InTy>> input, string query_name)
{
    auto in_st = input[0].st;
    auto in_et = input[input.size() - 1].et;

    auto true_out = query_fn(input);
    auto out_st = true_out[0].st;
    auto out_et = true_out[input.size() - 1].et;

    region_t in_reg;
    auto in_tl = vector<ival_t>(input.size());
    auto in_data = vector<InTy>(input.size());
    auto in_data_ptr = reinterpret_cast<char*>(in_data.data());
    init_region(&in_reg, in_st, get_buf_size(input.size()), in_tl.data(), in_data_ptr);
    for (size_t i = 0; i < input.size(); i++) {
        in_reg.et = input[i].et;
        in_reg.ei++;
        in_tl[in_reg.ei] = {input[i].st, static_cast<dur_t>(input[i].et - input[i].st)};
        in_data[in_reg.ei] = input[i].payload;
    }

    region_t out_reg;
    auto out_tl = vector<ival_t>(true_out.size());
    auto out_data = vector<OutTy>(true_out.size());
    auto out_data_ptr = reinterpret_cast<char*>(out_data.data());
    init_region(&out_reg, out_st, get_buf_size(true_out.size()), out_tl.data(), out_data_ptr);

    run_op(op, out_st, out_et, &out_reg, &in_reg, query_name);

    for (size_t i = 0; i < true_out.size(); i++) {
        auto true_st = true_out[i].st;
        auto true_et = true_out[i].et;
        auto true_payload = true_out[i].payload;
        auto out_st = out_tl[i].t;
        auto out_et = out_st + out_tl[i].d;
        auto out_payload = out_data[i];

        assert_eq(true_st, out_st);
        assert_eq(true_et, out_et);
        assert_eq(true_payload, out_payload);
    }
}

template<typename InTy, typename OutTy>
void unary_op_test(Op op, QueryFn<InTy, OutTy> query_fn, size_t len, int64_t dur, string query_name)
{
    std::srand(time(nullptr));

    vector<Event<InTy>> input(len);
    for (size_t i = 0; i < len; i++) {
        int64_t st = dur * i;
        int64_t et = st + dur;
        InTy payload = static_cast<InTy>(std::rand() / static_cast<double>(RAND_MAX / 10000));
        input[i] = {st, et, payload};
    }

    op_test<InTy, OutTy>(op, query_fn, input, query_name);
}

template<typename InTy, typename OutTy>
void select_test(function<Expr(Expr)> sel_expr, function<OutTy(InTy)> sel_fn, string query_name)
{
    size_t len = 1000;
    int64_t dur = 5;

    auto in_sym = _sym("in", tilt::Type(types::STRUCT<InTy>(), _iter("in")));
    auto sel_op = _Select(in_sym, sel_expr);

    auto sel_query_fn = [sel_fn] (vector<Event<InTy>> in) {
        vector<Event<OutTy>> out;

        for (size_t i = 0; i < in.size(); i++) {
            out.push_back({in[i].st, in[i].et, sel_fn(in[i].payload)});
        }

        return move(out);
    };

    unary_op_test<InTy, OutTy>(sel_op, sel_query_fn, len, dur, query_name);
}

void add_test()
{
    select_test<int32_t, int32_t>(
        [] (Expr s) { return _add(s, _i32(10)); },
        [] (int32_t s) { return s + 10; }, "iadd");
    select_test<float, float>(
        [] (Expr s) { return _add(s, _f32(5)); },
        [] (float s) { return s + 5.0; }, "fadd");
}

void sub_test()
{
    select_test<int32_t, int32_t>(
        [] (Expr s) { return _sub(s, _i32(10)); },
        [] (int32_t s) { return s - 10; }, "isub");
    select_test<float, float>(
        [] (Expr s) { return _sub(s, _f32(15)); },
        [] (float s) { return s - 15.0; }, "fsub");
}

void mul_test()
{
    select_test<int32_t, int32_t>(
        [] (Expr s) { return _mul(s, _i32(10)); },
        [] (int32_t s) { return s * 10; }, "imul");
    select_test<float, float>(
        [] (Expr s) { return _mul(s, _f32(10)); },
        [] (float s) { return s * 10.0f; }, "fmul");
}

void div_test()
{
    select_test<int32_t, int32_t>(
        [] (Expr s) { return _div(s, _i32(10)); },
        [] (int32_t s) { return s / 10; }, "idiv");
    select_test<uint32_t, uint32_t>(
        [] (Expr s) { return _div(s, _u32(10)); },
        [] (uint32_t s) { return s / 10u; }, "udiv");
    select_test<float, float>(
        [] (Expr s) { return _div(s, _f32(10)); },
        [] (float s) { return s / 10.0f; }, "fdiv");
}

void mod_test()
{
    select_test<int32_t, int32_t>(
        [] (Expr s) { return _mod(s, _i32(10)); },
        [] (int32_t s) { return s % 10; }, "imod");
    select_test<uint32_t, uint32_t>(
        [] (Expr s) { return _mod(s, _u32(10)); },
        [] (uint32_t s) { return s % 10u; }, "umod");
}

void max_test()
{
    select_test<int32_t, int32_t>(
        [] (Expr s) { return _max(s, _i32(10)); },
        [] (int32_t s) { return std::max(s, 10); }, "imax");
    select_test<uint32_t, uint32_t>(
        [] (Expr s) { return _max(s, _u32(10)); },
        [] (uint32_t s) { return std::max(s, 10u); }, "umax");
    select_test<float, float>(
        [] (Expr s) { return _max(s, _f32(10)); },
        [] (float s) { return std::max(s, 10.0f); }, "fmax");
}

void min_test()
{
    select_test<int32_t, int32_t>(
        [] (Expr s) { return _min(s, _i32(10)); },
        [] (int32_t s) { return std::min(s, 10); }, "imin");
    select_test<uint32_t, uint32_t>(
        [] (Expr s) { return _min(s, _u32(10)); },
        [] (uint32_t s) { return std::min(s, 10u); }, "umin");
    select_test<float, float>(
        [] (Expr s) { return _min(s, _f32(10)); },
        [] (float s) { return std::min(s, 10.0f); }, "fmin");
}

void neg_test()
{
    select_test<int32_t, int32_t>(
        [] (Expr s) { return _neg(s); },
        [] (int32_t s) { return -s; }, "ineg");
    select_test<float, float>(
        [] (Expr s) { return _neg(s); },
        [] (float s) { return -s; }, "fneg");
    select_test<double, double>(
        [] (Expr s) { return _neg(s); },
        [] (double s) { return -s; }, "dneg");
}

void sqrt_test()
{
    select_test<float, float>(
        [] (Expr s) { return _sqrt(s); },
        [] (float s) { return std::sqrt(s); }, "fsqrt");
    select_test<double, double>(
        [] (Expr s) { return _sqrt(s); },
        [] (double s) { return std::sqrt(s); }, "dsqrt");
}

void pow_test()
{
    select_test<float, float>(
        [] (Expr s) { return _pow(s, _f32(2)); },
        [] (float s) { return std::pow(s, 2); }, "fpow");
    select_test<double, double>(
        [] (Expr s) { return _pow(s, _f64(2)); },
        [] (double s) { return std::pow(s, 2); }, "dpow");
}

void ceil_test()
{
    select_test<float, float>(
        [] (Expr s) { return _ceil(s); },
        [] (float s) { return std::ceil(s); }, "fceil");
    select_test<double, double>(
        [] (Expr s) { return _ceil(s); },
        [] (double s) { return std::ceil(s); }, "dceil");
}

void floor_test()
{
    select_test<float, float>(
        [] (Expr s) { return _floor(s); },
        [] (float s) { return std::floor(s); }, "ffloor");
    select_test<double, double>(
        [] (Expr s) { return _floor(s); },
        [] (double s) { return std::floor(s); }, "dfloor");
}

void abs_test()
{
    select_test<float, float>(
        [] (Expr s) { return _abs(s); },
        [] (float s) { return std::abs(s); }, "fabs");
    select_test<double, double>(
        [] (Expr s) { return _abs(s); },
        [] (double s) { return std::abs(s); }, "dabs");
    select_test<int32_t, int32_t>(
        [] (Expr s) { return _abs(s); },
        [] (int32_t s) { return std::abs(s); }, "iabs");
}

void cast_test()
{
    select_test<int32_t, float>(
        [] (Expr s) { return _cast(types::FLOAT32, s); },
        [] (int32_t s) { return static_cast<float>(s); }, "sitofp");
    select_test<uint32_t, float>(
        [] (Expr s) { return _cast(types::FLOAT32, s); },
        [] (uint32_t s) { return static_cast<float>(s); }, "uitofp");
    select_test<float, int32_t>(
        [] (Expr s) { return _cast(types::INT32, s); },
        [] (float s) { return static_cast<int32_t>(s); }, "fptosi");
    select_test<float, uint32_t>(
        [] (Expr s) { return _cast(types::UINT32, s); },
        [] (float s) { return static_cast<uint32_t>(s); }, "fptoui");
    select_test<int8_t, int32_t>(
        [] (Expr s) { return _cast(types::INT32, s); },
        [] (int8_t s) { return static_cast<int32_t>(s); }, "int8toint32");
}

void moving_sum_test()
{
    size_t len = 30;
    int64_t dur = 1;
    int64_t w = 10;

    auto in_sym = _sym("in", tilt::Type(types::INT32, _iter("in")));
    auto mov_op = _MovingSum(in_sym, dur, w);

    auto mov_query_fn = [w] (vector<Event<int32_t>> in) {
        vector<Event<int32_t>> out(in.size());

        for (int i = 0; i < in.size(); i++) {
            auto out_i = i - 1;
            auto tail_i = i - w;
            auto payload = in[i].payload
                    - ((tail_i < 0) ? 0 : in[tail_i].payload)
                    + ((out_i < 0) ? 0 : out[out_i].payload);
            out[i] = {in[i].st, in[i].et, payload};
        }

        return move(out);
    };

    unary_op_test<int32_t, int32_t>(mov_op, mov_query_fn, len, dur, "moving_sum");
}
