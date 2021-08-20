#include <iostream>
#include <memory>
#include <cstdlib>
#include <chrono>
#include <numeric>

#include "tilt/codegen/printer.h"
#include "tilt/codegen/loopgen.h"
#include "tilt/codegen/llvmgen.h"
#include "tilt/codegen/vinstr.h"
#include "tilt/engine/engine.h"

#include "quilt.h"

using namespace std;
using namespace std::chrono;

Op Pair(_sym in, int64_t iperiod)
{
    auto ev = in[_pt(0)];
    auto ev_sym = _sym("ev", ev);
    auto sv = in[_pt(-iperiod)];
    auto sv_sym = _sym("sv", sv);
    auto sv_val = _ifelse(_exists(sv_sym), sv_sym, _f32(0));
    auto sv_val_sym = _sym("sv_val", sv_val);
    auto beat = _beat(_iter(0, iperiod));
    auto et = _cast(types::INT64, beat[_pt(0)]);
    auto et_sym = _sym("et", et);
    auto st = et_sym - _i64(iperiod);
    auto st_sym = _sym("st", st);
    auto res = _new(vector<Expr>{st_sym, sv_val_sym, et_sym, ev_sym});
    auto res_sym = _sym("res", res);
    auto pair_op = _op(
        _iter(0, iperiod),
        Params{in, beat},
        SymTable{
            {st_sym, st},
            {sv_sym, sv},
            {sv_val_sym, sv_val},
            {et_sym, et},
            {ev_sym, ev},
            {res_sym, res},
        },
        _exists(ev_sym),
        res_sym);
    return pair_op;
}

Op Interpolate(_sym in, int64_t operiod)
{
    auto e = in[_pt(0)];
    auto e_sym = _sym("e", e);
    auto beat = _beat(_iter(0, operiod));
    auto t = _cast(types::INT64, beat[_pt(0)]);
    auto t_sym = _sym("t", t);
    auto st = e_sym << 0;
    auto sv = e_sym << 1;
    auto et = e_sym << 2;
    auto ev = e_sym << 3;
    auto res = (((ev - sv) * _cast(types::FLOAT32, t_sym - st)) / _cast(types::FLOAT32, et - st)) + sv;
    auto res_sym = _sym("res", res);
    auto inter_op = _op(
        _iter(0, operiod),
        Params{in, beat},
        SymTable{
            {e_sym, e},
            {t_sym, t},
            {res_sym, res},
        },
        _exists(e_sym),
        res_sym);
    return inter_op;
}

Op Resample(_sym in, int64_t iperiod, int64_t operiod, int64_t len)
{
    auto win_size = lcm(iperiod, operiod) * len;
    auto win = in[_win(-win_size, 0)];
    auto win_sym = _sym("win", in);
    auto pair = Pair(win_sym, iperiod);
    auto pair_sym = _sym("pair", pair);
    auto inter = Interpolate(pair_sym, operiod);
    auto inter_sym = _sym("inter", inter);
    auto resample_op = _op(
        _iter(0, win_size),
        Params{in},
        SymTable{
            {win_sym, win},
            {pair_sym, pair},
            {inter_sym, inter},
        },
        _true(),
        inter_sym);
    return resample_op;
}

int main(int argc, char** argv)
{
    int dlen = (argc > 1) ? atoi(argv[1]) : 60;
    int len = (argc > 2) ? atoi(argv[2]) : 10;
    uint32_t dur = 5;
    auto iperiod = dur;
    auto operiod = 4;

    // input stream
    auto in_sym = _sym("in", tilt::Type(types::FLOAT32, _iter(0, -1)));

    auto query_op = Resample(in_sym, iperiod, operiod, 100);
    auto query_op_sym = _sym("query", query_op);
    cout << endl << "TiLT IR:" << endl;
    cout << IRPrinter::Build(query_op) << endl;

    auto loop = LoopGen::Build(query_op_sym, query_op.get());
    cout << endl << "Loop IR:" << endl;
    cout << IRPrinter::Build(loop);

    auto jit = ExecEngine::Get();
    auto& llctx = jit->GetCtx();
    auto llmod = LLVMGen::Build(loop, llctx);
    cout << endl << "LLVM IR:" << endl;
    cout << IRPrinter::Build(llmod.get()) << endl;

    jit->AddModule(move(llmod));

    auto loop_addr = (region_t* (*)(ts_t, ts_t, region_t*, region_t*)) jit->Lookup(loop->get_name());

    auto in_tl = new ival_t[dlen];
    auto in_data = new float[dlen];
    region_t in_reg;
    init_region(&in_reg, 0, get_buf_size(dlen), in_tl, reinterpret_cast<char*>(in_data));
    for (int i = 0; i < dlen; i++) {
        auto t = dur*i;
        in_reg.et = t;
        in_reg.ei++;
        in_tl[in_reg.ei] = {t, dur};
        in_data[in_reg.ei] = i%1000 + 1;
    }

    struct Data {
        int64_t st;
        float sv;
        int64_t et;
        float ev;

        string str() const
        {
            return "{"
                + to_string(st) + ","
                + to_string(sv) + ","
                + to_string(et) + ","
                + to_string(ev) + "}";
        }
    };
    auto out_tl = new ival_t[dlen];
    auto out_data = new float[dlen];
    region_t out_reg;
    init_region(&out_reg, 0, get_buf_size(dlen), out_tl, reinterpret_cast<char*>(out_data));

    cout << "Query execution: " << endl;
    auto start_time = high_resolution_clock::now();
    auto* res_reg = loop_addr(0, dur*dlen, &out_reg, &in_reg);
    auto end_time = high_resolution_clock::now();

    int out_count = dlen;
    if (argc == 1) {
        for (int i = 0; i < dlen; i++) {
            cout << "(" << in_tl[i].t << "," << in_tl[i].t + in_tl[i].d << ") " << in_data[i] << " -> "
                << "(" << out_tl[i].t << "," << out_tl[i].t + out_tl[i].d << ")" << out_data[i] << endl;
        }
    }

    auto time = duration_cast<microseconds>(end_time - start_time).count();
    cout << "Data size: " << out_count << " Time: " << time << endl;

    return 0;
}
