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

void run_op(Op op, ts_t st, ts_t et, region_t* out_reg, region_t* in_reg)
{
    auto op_sym = op->sym("query");
    auto loop = LoopGen::Build(op_sym, op.get());

    auto jit = ExecEngine::Get();
    auto& llctx = jit->GetCtx();

    auto llmod = LLVMGen::Build(loop, llctx);
    jit->AddModule(move(llmod));

    auto loop_addr = (region_t* (*)(ts_t, ts_t, region_t*, region_t*)) jit->Lookup(loop->get_name());

    loop_addr(st, et, out_reg, in_reg);
}

template<typename InTy, typename OutTy>
void op_test(Op op, QueryFn<InTy, OutTy> query_fn, vector<Event<InTy>> input)
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

    run_op(op, out_st, out_et, &out_reg, &in_reg);

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
void unary_op_test(Op op, QueryFn<InTy, OutTy> query_fn, size_t len, int64_t dur)
{
    std::srand(time(nullptr));

    vector<Event<InTy>> input(len);
    for (size_t i = 0; i < len; i++) {
        int64_t st = dur * i;
        int64_t et = st + dur;
        InTy payload = static_cast<InTy>(std::rand() / static_cast<double>(RAND_MAX / 10000));
        input[i] = {st, et, payload};
    }

    op_test<InTy, OutTy>(op, query_fn, input);
}

template<typename InTy, typename OutTy>
void select_test(function<Expr(Expr)> sel_expr, function<OutTy(InTy)> sel_fn)
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

    unary_op_test<InTy, OutTy>(sel_op, sel_query_fn, len, dur);
}

void iadd_test()
{
    select_test<int32_t, int32_t>(
        [] (Expr s) { return _add(s, _i32(10)); },
        [] (int32_t s) { return s + 10; });
}

void fadd_test()
{
    select_test<float, float>(
        [] (Expr s) { return _add(s, _f32(5)); },
        [] (float s) { return s + 5.0; });
}

void isub_test()
{
    select_test<int32_t, int32_t>(
        [] (Expr s) { return _sub(s, _i32(10)); },
        [] (int32_t s) { return s - 10; });
}

void fsub_test()
{
    select_test<float, float>(
        [] (Expr s) { return _sub(s, _f32(15)); },
        [] (float s) { return s - 15.0; });
}

void imul_test()
{
    select_test<int32_t, int32_t>(
        [] (Expr s) { return _mul(s, _i32(10)); },
        [] (int32_t s) { return s * 10; });
}

void fmul_test()
{
    select_test<float, float>(
        [] (Expr s) { return _mul(s, _f32(10)); },
        [] (float s) { return s * 10.0f; });
}

void idiv_test()
{
    select_test<int32_t, int32_t>(
        [] (Expr s) { return _div(s, _i32(10)); },
        [] (int32_t s) { return s / 10; });
}

void udiv_test()
{
    select_test<uint32_t, uint32_t>(
        [] (Expr s) { return _div(s, _u32(10)); },
        [] (uint32_t s) { return s / 10u; });
}

void fdiv_test()
{
    select_test<float, float>(
        [] (Expr s) { return _div(s, _f32(10)); },
        [] (float s) { return s / 10.0f; });
}

void imod_test()
{
    select_test<int32_t, int32_t>(
        [] (Expr s) { return _mod(s, _i32(10)); },
        [] (int32_t s) { return s % 10; });
}

void umod_test()
{
    select_test<uint32_t, uint32_t>(
        [] (Expr s) { return _mod(s, _u32(10)); },
        [] (uint32_t s) { return s % 10u; });
}

void imax_test()
{
    select_test<int32_t, int32_t>(
        [] (Expr s) { return _max(s, _i32(10)); },
        [] (int32_t s) { return std::max(s, 10); });
}

void umax_test()
{
    select_test<uint32_t, uint32_t>(
        [] (Expr s) { return _max(s, _u32(10)); },
        [] (uint32_t s) { return std::max(s, 10u); });
}

void fmax_test()
{
    select_test<float, float>(
        [] (Expr s) { return _max(s, _f32(10)); },
        [] (float s) { return std::max(s, 10.0f); });
}

void imin_test()
{
    select_test<int32_t, int32_t>(
        [] (Expr s) { return _min(s, _i32(10)); },
        [] (int32_t s) { return std::min(s, 10); });
}

void umin_test()
{
    select_test<uint32_t, uint32_t>(
        [] (Expr s) { return _min(s, _u32(10)); },
        [] (uint32_t s) { return std::min(s, 10u); });
}

void fmin_test()
{
    select_test<float, float>(
        [] (Expr s) { return _min(s, _f32(10)); },
        [] (float s) { return std::min(s, 10.0f); });
}

void ineg_test()
{
    select_test<int32_t, int32_t>(
        [] (Expr s) { return _neg(s); },
        [] (int32_t s) { return -s; });
}

void fneg_test()
{
    select_test<float, float>(
        [] (Expr s) { return _neg(s); },
        [] (float s) { return -s; });
}

void dneg_test()
{
    select_test<double, double>(
        [] (Expr s) { return _neg(s); },
        [] (double s) { return -s; });
}

void fsqrt_test()
{
    select_test<float, float>(
        [] (Expr s) { return _sqrt(s); },
        [] (float s) { return std::sqrt(s); });
}

void dsqrt_test()
{
    select_test<double, double>(
        [] (Expr s) { return _sqrt(s); },
        [] (double s) { return std::sqrt(s); });
}

void fpow_test()
{
    select_test<float, float>(
        [] (Expr s) { return _pow(s, _f32(2)); },
        [] (float s) { return std::pow(s, 2); });
}

void dpow_test()
{
    select_test<double, double>(
        [] (Expr s) { return _pow(s, _f64(2)); },
        [] (double s) { return std::pow(s, 2); });
}

void fceil_test()
{
    select_test<float, float>(
        [] (Expr s) { return _ceil(s); },
        [] (float s) { return std::ceil(s); });
}

void dceil_test()
{
    select_test<double, double>(
        [] (Expr s) { return _ceil(s); },
        [] (double s) { return std::ceil(s); });
}

void ffloor_test()
{
    select_test<float, float>(
        [] (Expr s) { return _floor(s); },
        [] (float s) { return std::floor(s); });
}

void dfloor_test()
{
    select_test<double, double>(
        [] (Expr s) { return _floor(s); },
        [] (double s) { return std::floor(s); });
}

void fabs_test()
{
    select_test<float, float>(
        [] (Expr s) { return _abs(s); },
        [] (float s) { return std::abs(s); });
}

void dabs_test()
{
    select_test<double, double>(
        [] (Expr s) { return _abs(s); },
        [] (double s) { return std::abs(s); });
}

void iabs_test()
{
    select_test<int32_t, int32_t>(
        [] (Expr s) { return _abs(s); },
        [] (int32_t s) { return std::abs(s); });
}

void sitofp_test()
{
    select_test<int32_t, float>(
        [] (Expr s) { return _cast(types::FLOAT32, s); },
        [] (int32_t s) { return static_cast<float>(s); });
}

void uitofp_test()
{
    select_test<uint32_t, float>(
        [] (Expr s) { return _cast(types::FLOAT32, s); },
        [] (uint32_t s) { return static_cast<float>(s); });
}

void fptosi_test()
{
    select_test<float, int32_t>(
        [] (Expr s) { return _cast(types::INT32, s); },
        [] (float s) { return static_cast<int32_t>(s); });
}

void fptoui_test()
{
    select_test<float, uint32_t>(
        [] (Expr s) { return _cast(types::UINT32, s); },
        [] (float s) { return static_cast<uint32_t>(s); });
}

void int8toint32_test()
{
    select_test<int8_t, int32_t>(
        [] (Expr s) { return _cast(types::INT32, s); },
        [] (int8_t s) { return static_cast<int32_t>(s); });
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

    unary_op_test<int32_t, int32_t>(mov_op, mov_query_fn, len, dur);
}
