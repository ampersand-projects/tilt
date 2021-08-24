#include <utility>
#include <cmath>
#include <algorithm>
#include <string>
#include <numeric>

#include "tilt/codegen/loopgen.h"
#include "tilt/codegen/llvmgen.h"
#include "tilt/codegen/vinstr.h"
#include "tilt/engine/engine.h"

#include "test_base.h"

using namespace tilt;
using namespace tilt::tilder;

void run_op(string query_name, Op op, ts_t st, ts_t et, region_t* out_reg, region_t* in_reg)
{
    auto op_sym = _sym(query_name, op);
    auto loop = LoopGen::Build(op_sym, op.get());

    auto jit = ExecEngine::Get();
    auto& llctx = jit->GetCtx();

    auto llmod = LLVMGen::Build(loop, llctx);
    jit->AddModule(move(llmod));

    auto loop_addr = (region_t* (*)(ts_t, ts_t, region_t*, region_t*)) jit->Lookup(loop->get_name());

    loop_addr(st, et, out_reg, in_reg);
}

template<typename InTy, typename OutTy>
void op_test(string query_name, Op op, QueryFn<InTy, OutTy> query_fn, vector<Event<InTy>> input)
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

    run_op(query_name, op, out_st, out_et, &out_reg, &in_reg);

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
void unary_op_test(string query_name, Op op, QueryFn<InTy, OutTy> query_fn, size_t len, int64_t dur)
{
    std::srand(time(nullptr));

    vector<Event<InTy>> input(len);
    for (size_t i = 0; i < len; i++) {
        int64_t st = dur * i;
        int64_t et = st + dur;
        InTy payload = static_cast<InTy>(std::rand() / static_cast<double>(RAND_MAX / 10000));
        input[i] = {st, et, payload};
    }

    op_test<InTy, OutTy>(query_name, op, query_fn, input);
}

template<typename InTy, typename OutTy>
void select_test(string query_name, function<Expr(Expr)> sel_expr, function<OutTy(InTy)> sel_fn)
{
    size_t len = 1000;
    int64_t dur = 5;

    auto in_sym = _sym("in", tilt::Type(types::STRUCT<InTy>(), _iter(0, -1)));
    auto sel_op = _Select(in_sym, sel_expr);

    auto sel_query_fn = [sel_fn] (vector<Event<InTy>> in) {
        vector<Event<OutTy>> out;

        for (size_t i = 0; i < in.size(); i++) {
            out.push_back({in[i].st, in[i].et, sel_fn(in[i].payload)});
        }

        return move(out);
    };

    unary_op_test<InTy, OutTy>(query_name, sel_op, sel_query_fn, len, dur);
}

void add_test()
{
    select_test<int32_t, int32_t>("iadd",
        [] (Expr s) { return _add(s, _i32(10)); },
        [] (int32_t s) { return s + 10; });
    select_test<float, float>("fadd",
        [] (Expr s) { return _add(s, _f32(5)); },
        [] (float s) { return s + 5.0; });
}

void sub_test()
{
    select_test<int32_t, int32_t>("isub",
        [] (Expr s) { return _sub(s, _i32(10)); },
        [] (int32_t s) { return s - 10; });
    select_test<float, float>("fsub",
        [] (Expr s) { return _sub(s, _f32(15)); },
        [] (float s) { return s - 15.0; });
}

void mul_test()
{
    select_test<int32_t, int32_t>("imul",
        [] (Expr s) { return _mul(s, _i32(10)); },
        [] (int32_t s) { return s * 10; });
    select_test<float, float>("fmul",
        [] (Expr s) { return _mul(s, _f32(10)); },
        [] (float s) { return s * 10.0f; });
}

void div_test()
{
    select_test<int32_t, int32_t>("idiv",
        [] (Expr s) { return _div(s, _i32(10)); },
        [] (int32_t s) { return s / 10; });
    select_test<uint32_t, uint32_t>("udiv",
        [] (Expr s) { return _div(s, _u32(10)); },
        [] (uint32_t s) { return s / 10u; });
    select_test<float, float>("fdiv",
        [] (Expr s) { return _div(s, _f32(10)); },
        [] (float s) { return s / 10.0f; });
}

void mod_test()
{
    select_test<int32_t, int32_t>("imod",
        [] (Expr s) { return _mod(s, _i32(10)); },
        [] (int32_t s) { return s % 10; });
    select_test<uint32_t, uint32_t>("umod",
        [] (Expr s) { return _mod(s, _u32(10)); },
        [] (uint32_t s) { return s % 10u; });
}

void max_test()
{
    select_test<int32_t, int32_t>("imax",
        [] (Expr s) { return _max(s, _i32(10)); },
        [] (int32_t s) { return std::max(s, 10); });
    select_test<uint32_t, uint32_t>("umax",
        [] (Expr s) { return _max(s, _u32(10)); },
        [] (uint32_t s) { return std::max(s, 10u); });
    select_test<float, float>("fmax",
        [] (Expr s) { return _max(s, _f32(10)); },
        [] (float s) { return std::max(s, 10.0f); });
}

void min_test()
{
    select_test<int32_t, int32_t>("imin",
        [] (Expr s) { return _min(s, _i32(10)); },
        [] (int32_t s) { return std::min(s, 10); });
    select_test<uint32_t, uint32_t>("umin",
        [] (Expr s) { return _min(s, _u32(10)); },
        [] (uint32_t s) { return std::min(s, 10u); });
    select_test<float, float>("fmin",
        [] (Expr s) { return _min(s, _f32(10)); },
        [] (float s) { return std::min(s, 10.0f); });
}

void neg_test()
{
    select_test<int32_t, int32_t>("ineg",
        [] (Expr s) { return _neg(s); },
        [] (int32_t s) { return -s; });
    select_test<float, float>("fneg",
        [] (Expr s) { return _neg(s); },
        [] (float s) { return -s; });
    select_test<double, double>("dneg",
        [] (Expr s) { return _neg(s); },
        [] (double s) { return -s; });
}

void sqrt_test()
{
    select_test<float, float>("fsqrt",
        [] (Expr s) { return _sqrt(s); },
        [] (float s) { return std::sqrt(s); });
    select_test<double, double>("dsqrt",
        [] (Expr s) { return _sqrt(s); },
        [] (double s) { return std::sqrt(s); });
}

void pow_test()
{
    select_test<float, float>("fpow",
        [] (Expr s) { return _pow(s, _f32(2)); },
        [] (float s) { return std::pow(s, 2); });
    select_test<double, double>("dpow",
        [] (Expr s) { return _pow(s, _f64(2)); },
        [] (double s) { return std::pow(s, 2); });
}

void ceil_test()
{
    select_test<float, float>("fceil",
        [] (Expr s) { return _ceil(s); },
        [] (float s) { return std::ceil(s); });
    select_test<double, double>("dceil",
        [] (Expr s) { return _ceil(s); },
        [] (double s) { return std::ceil(s); });
}

void floor_test()
{
    select_test<float, float>("ffloor",
        [] (Expr s) { return _floor(s); },
        [] (float s) { return std::floor(s); });
    select_test<double, double>("dfloor",
        [] (Expr s) { return _floor(s); },
        [] (double s) { return std::floor(s); });
}

void abs_test()
{
    select_test<float, float>("fabs",
        [] (Expr s) { return _abs(s); },
        [] (float s) { return std::abs(s); });
    select_test<double, double>("dabs",
        [] (Expr s) { return _abs(s); },
        [] (double s) { return std::abs(s); });
    select_test<int32_t, int32_t>("iabs",
        [] (Expr s) { return _abs(s); },
        [] (int32_t s) { return std::abs(s); });
}

void cast_test()
{
    select_test<int32_t, float>("sitofp",
        [] (Expr s) { return _cast(types::FLOAT32, s); },
        [] (int32_t s) { return static_cast<float>(s); });
    select_test<uint32_t, float>("uitofp",
        [] (Expr s) { return _cast(types::FLOAT32, s); },
        [] (uint32_t s) { return static_cast<float>(s); });
    select_test<float, int32_t>("fptosi",
        [] (Expr s) { return _cast(types::INT32, s); },
        [] (float s) { return static_cast<int32_t>(s); });
    select_test<float, uint32_t>("fptoui",
        [] (Expr s) { return _cast(types::UINT32, s); },
        [] (float s) { return static_cast<uint32_t>(s); });
    select_test<int8_t, int32_t>("int8toint32",
        [] (Expr s) { return _cast(types::INT32, s); },
        [] (int8_t s) { return static_cast<int32_t>(s); });
}

void moving_sum_test()
{
    size_t len = 30;
    int64_t dur = 1;
    int64_t w = 10;

    auto in_sym = _sym("in", tilt::Type(types::INT32, _iter(0, -1)));
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

    unary_op_test<int32_t, int32_t>("moving_sum", mov_op, mov_query_fn, len, dur);
}

void norm_test()
{
    size_t len = 1000;
    int64_t dur = 1;
    int64_t w = 10;

    auto in_sym = _sym("in", tilt::Type(types::FLOAT32, _iter("in")));
    auto norm_op = _Norm(in_sym, w);

    auto norm_query_fn = [w] (vector<Event<float>> in) {
        vector<Event<float>> out(in.size());
        size_t num_windows = ceil(in.size() / 10.0);

        for (size_t i = 0; i < num_windows; i++) {
            float sum = 0.0, mean, variance = 0.0, std_dev;

            for (size_t j = 0; j < w; j++) {
                sum += in[i * w + j].payload;
            }
            mean = sum / w;
            for (size_t j = 0; j < w; j++) {
                variance += pow(in[i * w + j].payload - mean, 2);
            }
            std_dev = sqrt(variance / w);

            for (size_t j = 0; j < w; j++) {
                size_t idx = i * w + j;
                float z_score = (in[idx].payload - mean) / std_dev;
                out[idx] = {in[idx].st, in[idx].et, z_score};
            }
        }

        return move(out);
    };

    unary_op_test<float, float>("norm", norm_op, norm_query_fn, len, dur);
}
