#include "tilt/base/type.h"
#include "tilt/codegen/llvmgen.h"

#include "llvm/IR/Function.h"

using namespace std;
using namespace tilt;
using namespace llvm;

llvm::Function* LLVMGen::get_func(
    const string name, llvm::Type* ret_type, vector<llvm::Type*> arg_types)
{
    auto fn = ctx.llmodule()->getFunction(name);
    if (!fn) {
        auto fn_type = FunctionType::get(ret_type, arg_types, false);
        fn = Function::Create(fn_type, Function::ExternalLinkage, name, ctx.llmodule());
    }

    return fn;
}

llvm::Type* LLVMGen::lltype(const PrimitiveType& btype)
{
    switch (btype)
    {
    case PrimitiveType::BOOL:
        return llvm::Type::getInt1Ty(ctx.llcontext());
    case PrimitiveType::CHAR:
        return llvm::Type::getInt8Ty(ctx.llcontext());
    case PrimitiveType::INT8:
    case PrimitiveType::UINT8:
        return llvm::Type::getInt8Ty(ctx.llcontext());
    case PrimitiveType::INT16:
    case PrimitiveType::UINT16:
        return llvm::Type::getInt16Ty(ctx.llcontext());
    case PrimitiveType::INT32:
    case PrimitiveType::UINT32:
        return llvm::Type::getInt32Ty(ctx.llcontext());
    case PrimitiveType::INT64:
    case PrimitiveType::UINT64:
        return llvm::Type::getInt64Ty(ctx.llcontext());
    case PrimitiveType::FLOAT32:
        return llvm::Type::getFloatTy(ctx.llcontext());
    case PrimitiveType::FLOAT64:
        return llvm::Type::getDoubleTy(ctx.llcontext());
    case PrimitiveType::TIMESTAMP:
        return llvm::Type::getInt64Ty(ctx.llcontext());
    case PrimitiveType::TIME:
        return llvm::Type::getInt64Ty(ctx.llcontext());
    case PrimitiveType::INDEX:
        return llvm::StructType::get(
            ctx.llcontext(),
            {
                lltype(types::INT32),                           // index
                lltype(types::TIME),                            // time
            },
            "Index"
        );
    case PrimitiveType::UNKNOWN:
    default:
        return nullptr;
    }
}

llvm::Type* LLVMGen::lltype(const vector<PrimitiveType>& btypes, const bool is_ptr)
{
    vector<llvm::Type*> lltypes;
    for (auto btype: btypes) {
        lltypes.push_back(lltype(btype));
    }

    llvm::Type* type;
    if (btypes.size() > 1) {
        type = StructType::get(ctx.llcontext(), lltypes);
    } else {
        type = lltypes[0];
    }

    if (is_ptr) {
        type = PointerType::get(type, 0);
    }

    return type;
}

llvm::Type* LLVMGen::lltype(const DataType& dtype)
{
    return lltype(dtype.ptypes, dtype.is_ptr);
}

llvm::Type* LLVMGen::lltype(const Type& type)
{
    if (type.isLStream()) {
        auto reg_type = StructType::get(
            ctx.llcontext(),
            {
                lltype(types::INDEX),                           // start index
                lltype(types::INDEX),                           // end index
                lltype(types::INT32),                           // event type size
                lltype(types::INDEX.ptypes, true),              // timeline array
                lltype(types::INT8.ptypes, true),               // data buffer
            },
            "Region"
        );
        return PointerType::get(reg_type, 0);
    } else {
        return lltype(type.dtype);
    }
}

void LLVMGen::Visit(const Symbol& sym)
{
    ctx.val = ctx.sym_tbl[const_cast<Symbol*>(&sym)];
}

void LLVMGen::Visit(const IConst&) {}
void LLVMGen::Visit(const UConst&) {}
void LLVMGen::Visit(const FConst&) {}
void LLVMGen::Visit(const BConst&) {}
void LLVMGen::Visit(const CConst&) {}
void LLVMGen::Visit(const TConst&) {}
void LLVMGen::Visit(const Add&) {}
void LLVMGen::Visit(const Sub&) {}
void LLVMGen::Visit(const Max&) {}
void LLVMGen::Visit(const Min&) {}
void LLVMGen::Visit(const Now&) {}
void LLVMGen::Visit(const Exists&) {}
void LLVMGen::Visit(const Equals&) {}
void LLVMGen::Visit(const Not&) {}
void LLVMGen::Visit(const And&) {}
void LLVMGen::Visit(const Or&) {}
void LLVMGen::Visit(const True&) {}
void LLVMGen::Visit(const False&) {}
void LLVMGen::Visit(const LessThan&) {}
void LLVMGen::Visit(const LessThanEqual&) {}
void LLVMGen::Visit(const GreaterThan&) {}
void LLVMGen::Visit(const AggExpr&) {}

void LLVMGen::Visit(const GetTime& get_time)
{
    auto idx_val = eval(get_time.idx);
    ctx.val = ctx.builder->CreateExtractValue(idx_val, 1);
}

void LLVMGen::Visit(const Fetch& fetch)
{
    auto reg_val = eval(fetch.reg);
    auto idx_val = eval(fetch.idx);

    auto get_data_addr_fn = get_func(
            "get_data_addr_fn",
            lltype({PrimitiveType::INT8}, true),
            {lltype(fetch.reg->type), lltype(fetch.idx->type)}
        );
    auto addr = ctx.builder->CreateCall(get_data_addr_fn, {reg_val, idx_val});
    auto data_ptr = ctx.builder->CreateBitCast(addr, lltype(fetch.reg->type.dtype));
    ctx.val = ctx.builder->CreateLoad(data_ptr);
}

void LLVMGen::Visit(const Advance& adv)
{
    /*
    auto reg_val = eval(adv.reg);
    auto idx_val = eval(adv.idx);
    auto t_val = eval(adv.time);
    */
}

void LLVMGen::Visit(const Next&) {}

void LLVMGen::Visit(const GetStartIdx& start_idx)
{
    auto reg_val = eval(start_idx.reg);
    ctx.val = ctx.builder->CreateExtractValue(reg_val, 0);
}

void LLVMGen::Visit(const CommitData& commit)
{
    /*
    auto reg_val = eval(commit.reg);
    auto t = eval(commit.time);
    auto data = eval(commit.data);

    vector<llvm::Type*> args_type = {lltype(commit.reg->type), lltype(commit.time->type), lltype(type)};
    auto fn_type = FunctionType::get(lltype(commit.reg->type), args_type, false);
    auto fn = Function::Create(fn_type, Function::ExternalLinkage, "commit", ctx.llmodule());
    ctx.val = ctx.builder->CreateCall(fn, {reg_val, t, data});
    */
}

void LLVMGen::Visit(const CommitNull& commit)
{
    auto reg_val = eval(commit.reg);
    auto t = eval(commit.time);

    vector<llvm::Type*> args_type = {lltype(commit.reg->type), lltype(commit.time->type)};
    auto fn_type = FunctionType::get(lltype(commit.reg->type), args_type, false);
    auto fn = Function::Create(fn_type, Function::ExternalLinkage, "commit", ctx.llmodule());
    ctx.val = ctx.builder->CreateCall(fn, {reg_val, t});
}

void LLVMGen::Visit(const IfElse& ifelse)
{
    auto cond = eval(ifelse.cond);
    auto true_body = eval(ifelse.true_body);
    auto false_body = eval(ifelse.false_body);
    ctx.val = ctx.builder->CreateSelect(cond, true_body, false_body);
}

void LLVMGen::Visit(const Loop& loop)
{
#ifdef BLAH
    ctx.llmod = make_unique<Module>(loop.name, ctx.llcontext());
    ctx.builder = make_unique<IRBuilder<>>(ctx.llcontext());

    vector<llvm::Type*> args_type;
    for (const auto& input: loop.inputs) {
        args_type.push_back(lltype(input->type));
    }

    auto fn_type = FunctionType::get(lltype(loop.output->type), args_type, false);
    auto loop_fn = Function::Create(fn_type, Function::ExternalLinkage, loop.name, ctx.llmodule());

    auto preheader_bb = BasicBlock::Create(ctx.llcontext(), "preheader");
    auto header_bb = BasicBlock::Create(ctx.llcontext(), "header");
    auto body_bb = BasicBlock::Create(ctx.llcontext(), "body");
    auto end_bb = BasicBlock::Create(ctx.llcontext(), "end");
    auto exit_bb = BasicBlock::Create(ctx.llcontext(), "exit");

    for (size_t i = 0; i < loop.inputs.size(); i++) {
        sym(loop.inputs[i]) = loop_fn->getArg(i);
        sym(loop.inputs[i])->setName(loop.inputs[i]->name);
    }

    /* Initial values of base states */
    loop_fn->getBasicBlockList().push_back(preheader_bb);
    ctx.builder->SetInsertPoint(preheader_bb);
    auto t_base_init = eval(loop.t_state.init);
    auto output_base_init = eval(loop.output_state.init);
    map<SymPtr, llvm::Value*> idx_base_init;
    for (const auto& [idx, state]: loop.idx_states) {
        idx_base_init[state.base] = eval(state.init);
    }
    ctx.builder->CreateBr(header_bb);

    /* Phi nodes for base states */
    loop_fn->getBasicBlockList().push_back(header_bb);
    ctx.builder->SetInsertPoint(header_bb);
    auto t_base = ctx.builder->CreatePHI(lltype(loop.t_state.base->type), 2, loop.t_state.base->name);
    sym(loop.t_state.base) = t_base;
    t_base->addIncoming(t_base_init, preheader_bb);
    auto output_base = ctx.builder->CreatePHI(lltype(loop.output_state.base->type), 2, loop.output_state.base->name);
    sym(loop.output_state.base) = output_base;
    output_base->addIncoming(output_base_init, preheader_bb);
    for (const auto& [idx_base_sym, val]: idx_base_init) {
        auto idx_base = ctx.builder->CreatePHI(lltype(idx_base_sym->type), 2, idx_base_sym->name);
        sym(idx_base_sym) = idx_base;
        idx_base->addIncoming(val, preheader_bb);
    }

    /* Update timer */
    sym(loop.t) = eval(loop.t_state.update);
    auto exit_cond = eval(loop.exit_cond);
    ctx.builder->CreateCondBr(exit_cond, exit_bb, body_bb);

    loop_fn->getBasicBlockList().push_back(body_bb);
    ctx.builder->SetInsertPoint(body_bb);
    /* Update indices */
    for (const auto& [idx, state]: loop.idx_states) {
        sym(idx) = eval(state.update);
    }

    /* Set local variables */
    for (const auto& [var, expr]: loop.vars) {
        sym(var) = eval(expr);
    }

    /* Loop body */
    sym(loop.output) = eval(loop.output_state.update);
    ctx.builder->CreateBr(end_bb);

    /* Loop back to header */
    loop_fn->getBasicBlockList().push_back(end_bb);
    ctx.builder->SetInsertPoint(end_bb);
    t_base->addIncoming(sym(loop.t), end_bb);
    output_base->addIncoming(sym(loop.output), end_bb);
    for (const auto& [idx, state]: loop.idx_states) {
        auto idx_base = dyn_cast<PHINode>(sym(state.base));
        idx_base->addIncoming(sym(idx), end_bb);
    }
    ctx.builder->CreateBr(header_bb);

    /* Exit the loop */
    loop_fn->getBasicBlockList().push_back(exit_bb);
    ctx.builder->SetInsertPoint(exit_bb);
    ctx.builder->CreateRet(sym(loop.output));
    ctx.val = loop_fn;
#endif
}
