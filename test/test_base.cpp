#include <utility>

#include "test_base.h"

#include "tilt/codegen/loopgen.h"
#include "tilt/codegen/llvmgen.h"
#include "tilt/engine/engine.h"
#include "tilt/builder/tilder.h"

#include "gtest/gtest.h"

using namespace tilt;
using namespace tilt::tilder;

void run_op(Op op, int64_t st, int64_t et, region_t* out_reg, region_t* in_reg)
{
    auto op_sym = op->sym("query");
    auto loop = LoopGen::Build(op_sym, op.get());

    auto jit = ExecEngine::Get();
    auto& llctx = jit->GetCtx();

    auto llmod = LLVMGen::Build(loop, llctx);
    jit->AddModule(move(llmod));

    auto loop_addr = (region_t* (*)(int64_t, int64_t, region_t*, region_t*)) jit->Lookup(loop->GetName());
    auto init_region = (region_t* (*)(region_t*, int64_t, index_t*, char*)) jit->Lookup("init_region");

    init_region(out_reg, st, out_reg->tl, out_reg->data);

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
    in_reg.si = {in_st, 0};
    in_reg.ei = {in_et, (uint32_t) input.size() - 1};
    auto in_tl = vector<index_t>(input.size());
    auto in_data = vector<InTy>(input.size());
    for (size_t i = 0; i < input.size(); i++) {
        in_tl[i] = {input[i].st, (uint32_t) (input[i].et - input[i].st)};
        in_data[i] = input[i].payload;
    }
    in_reg.tl = in_tl.data();
    in_reg.data = reinterpret_cast<char*>(in_data.data());

    region_t out_reg;
    auto out_tl = vector<index_t>(true_out.size());
    auto out_data = vector<OutTy>(true_out.size());
    out_reg.tl = out_tl.data();
    out_reg.data = reinterpret_cast<char*>(out_data.data());

    run_op(op, out_st, out_et, &out_reg, &in_reg);

    for (size_t i = 0; i < true_out.size(); i++) {
        auto true_st = true_out[i].st;
        auto true_et = true_out[i].et;
        auto true_payload = true_out[i].payload;
        auto out_st = out_tl[i].t;
        auto out_et = out_st + out_tl[i].i;
        auto out_payload = out_data[i];

        ASSERT_EQ(true_st, out_st);
        ASSERT_EQ(true_et, out_et);
        ASSERT_EQ(true_payload, out_payload);
    }
}

Op Select(Sym in, function<Expr(Expr)> sel_expr)
{
    auto e = _elem(in, _pt(0));
    auto e_sym = e->sym("e");
    auto sel = sel_expr(_get(e_sym, 0));
    auto sel_sym = sel->sym("sel");
    auto sel_op = _op(
        _iter(0, 1),
        Params{ in },
        SymTable{ {e_sym, e}, {sel_sym, sel} },
        _exists(e_sym),
        sel_sym);
    return sel_op;
}

template<typename InTy, typename OutTy>
void select_test(function<Expr(Expr)> sel_expr, function<OutTy(InTy)> sel_fn)
{
    const size_t len = 1000;
    const unsigned int dur = 5;

    auto in_sym = _sym("in", tilt::Type(types::STRUCT<InTy>(), _iter("in")));
    vector<Event<int32_t>> input(len);
    for (size_t i = 0; i < len; i++) {
        int64_t st = dur * i;
        int64_t et = st + dur;
        int32_t payload = i;
        input[i] = {st, et, payload};
    }

    auto sel_query_fn = [sel_fn] (vector<Event<InTy>> in) {
        vector<Event<OutTy>> out;

        for (size_t i = 0; i < in.size(); i++) {
            out.push_back({in[i].st, in[i].et, sel_fn(in[i].payload)});
        }

        return move(out);
    };

    auto sel_op = Select(in_sym, sel_expr);
    op_test<InTy, OutTy>(sel_op, sel_query_fn, input);
}

void addop_test()
{
    select_test<int32_t, int32_t>(
        [] (Expr s) { return _add(s, _i32(10)); },
        [] (int32_t s) { return s + 10; });
}
