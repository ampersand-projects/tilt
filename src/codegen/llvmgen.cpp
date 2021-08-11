#include "tilt/base/type.h"
#include "tilt/codegen/llvmgen.h"

#include "llvm/IR/Function.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/InstrTypes.h"

using namespace tilt;
using namespace llvm;

Function* LLVMGen::llfunc(const string name, llvm::Type* ret_type, vector<llvm::Type*> arg_types)
{
    auto fn = llmod()->getFunction(name);
    if (!fn) {
        auto fn_type = FunctionType::get(ret_type, arg_types, false);
        fn = Function::Create(fn_type, Function::ExternalLinkage, name, llmod());
    }

    return fn;
}

Value* LLVMGen::llcall(const string name, llvm::Type* ret_type, vector<Value*> arg_vals)
{
    vector<llvm::Type*> arg_types;
    for (const auto& arg_val : arg_vals) {
        arg_types.push_back(arg_val->getType());
    }
    auto fn = llfunc(name, ret_type, arg_types);
    return builder()->CreateCall(fn, arg_vals);
}

Value* LLVMGen::llcall(const string name, llvm::Type* ret_type, vector<Expr> args)
{
    vector<Value*> arg_vals;
    for (const auto& arg : args) {
        arg_vals.push_back(eval(arg));
    }

    return llcall(name, ret_type, arg_vals);
}

Value* LLVMGen::llsizeof(llvm::Type* type)
{
    auto size = llmod()->getDataLayout().getTypeSizeInBits(type).getFixedSize();
    return ConstantInt::get(lltype(types::UINT32), size/8);
}

llvm::Type* LLVMGen::lltype(const DataType& dtype)
{
    switch (dtype.btype) {
        case BaseType::BOOL:
            return llvm::Type::getInt1Ty(llctx());
        case BaseType::CHAR:
            return llvm::Type::getInt8Ty(llctx());
        case BaseType::INT8:
        case BaseType::UINT8:
            return llvm::Type::getInt8Ty(llctx());
        case BaseType::INT16:
        case BaseType::UINT16:
            return llvm::Type::getInt16Ty(llctx());
        case BaseType::INT32:
        case BaseType::UINT32:
            return llvm::Type::getInt32Ty(llctx());
        case BaseType::INT64:
        case BaseType::UINT64:
            return llvm::Type::getInt64Ty(llctx());
        case BaseType::FLOAT32:
            return llvm::Type::getFloatTy(llctx());
        case BaseType::FLOAT64:
            return llvm::Type::getDoubleTy(llctx());
        case BaseType::TIME:
            return lltype(DataType(types::Converter<ts_t>::btype));
        case BaseType::INDEX:
            return lltype(DataType(types::Converter<idx_t>::btype));
        case BaseType::IVAL:
            return llmod()->getTypeByName("struct.ival_t");
        case BaseType::STRUCT: {
            vector<llvm::Type*> lltypes;
            for (auto dt : dtype.dtypes) {
                lltypes.push_back(lltype(dt));
            }
            return StructType::get(llctx(), lltypes);
        }
        case BaseType::PTR:
            return PointerType::get(lltype(dtype.dtypes[0]), 0);
        case BaseType::UNKNOWN:
        default:
            throw std::runtime_error("Invalid type");
    }
}

llvm::Type* LLVMGen::lltype(const Type& type)
{
    if (type.is_valtype()) {
        return lltype(type.dtype);
    } else {
        auto reg_type = llmod()->getTypeByName("struct.region_t");
        return PointerType::get(reg_type, 0);
    }
}

Value* LLVMGen::visit(const Symbol& symbol)
{
    auto sym_ptr = sym(get_sym(symbol));
    auto& m = *(ctx().out_sym_tbl);
    return m[sym_ptr];
}

Value* LLVMGen::visit(const IfElse& ifelse)
{
    auto loop_fn = builder()->GetInsertBlock()->getParent();
    auto then_bb = BasicBlock::Create(llctx(), "then");
    auto else_bb = BasicBlock::Create(llctx(), "else");
    auto merge_bb = BasicBlock::Create(llctx(), "merge");

    // Condition check
    auto cond = eval(ifelse.cond);
    builder()->CreateCondBr(cond, then_bb, else_bb);

    // Then block
    loop_fn->getBasicBlockList().push_back(then_bb);
    builder()->SetInsertPoint(then_bb);
    auto true_val = eval(ifelse.true_body);
    then_bb = builder()->GetInsertBlock();
    builder()->CreateBr(merge_bb);

    // Else block
    loop_fn->getBasicBlockList().push_back(else_bb);
    builder()->SetInsertPoint(else_bb);
    auto false_val = eval(ifelse.false_body);
    else_bb = builder()->GetInsertBlock();
    builder()->CreateBr(merge_bb);

    // Merge block
    loop_fn->getBasicBlockList().push_back(merge_bb);
    builder()->SetInsertPoint(merge_bb);
    auto merge_phi = builder()->CreatePHI(lltype(ifelse), 2);
    merge_phi->addIncoming(true_val, then_bb);
    merge_phi->addIncoming(false_val, else_bb);

    return merge_phi;
}

Value* LLVMGen::visit(const Select& select)
{
    auto cond = eval(select.cond);
    auto true_val = eval(select.true_body);
    auto false_val = eval(select.false_body);
    return builder()->CreateSelect(cond, true_val, false_val);
}

Value* LLVMGen::visit(const Get& get)
{
    auto input = eval(get.input);
    return builder()->CreateExtractValue(input, get.n);
}

Value* LLVMGen::visit(const New& _new)
{
    auto ptr = builder()->CreateAlloca(lltype(_new));

    for (size_t i = 0; i < _new.inputs.size(); i++) {
        auto val_ptr = builder()->CreateStructGEP(ptr, i);
        builder()->CreateStore(eval(_new.inputs[i]), val_ptr);
    }

    return builder()->CreateLoad(ptr);
}

Value* LLVMGen::visit(const ConstNode& cnst)
{
    switch (cnst.type.dtype.btype) {
        case BaseType::BOOL:
        case BaseType::CHAR:
        case BaseType::INT8:
        case BaseType::INT16:
        case BaseType::INT32:
        case BaseType::INT64:
        case BaseType::UINT8:
        case BaseType::UINT16:
        case BaseType::UINT32:
        case BaseType::UINT64:
        case BaseType::TIME:
        case BaseType::INDEX: return ConstantInt::get(lltype(cnst), cnst.val);
        case BaseType::FLOAT32:
        case BaseType::FLOAT64: return ConstantFP::get(lltype(cnst), cnst.val);
        default: throw std::runtime_error("Invalid constant type"); break;
    }
}

Value* LLVMGen::visit(const Cast& e)
{  
    Instruction::CastOps op = CastInst::getCastOpcode(eval(e.arg), e.arg->type.dtype.is_signed(), lltype(e.type.dtype), e.type.dtype.is_signed());
    return builder()->CreateCast(op, eval(e.arg), lltype(e.type.dtype));
    }

Value* LLVMGen::visit(const NaryExpr& e)
{
    switch (e.op) {
        case MathOp::ADD: {
            if (e.type.dtype.is_float()) {
                return builder()->CreateFAdd(eval(e.arg(0)), eval(e.arg(1)));
            } else {
                return builder()->CreateAdd(eval(e.arg(0)), eval(e.arg(1)));
            }
        }
        case MathOp::SUB: {
            if (e.type.dtype.is_float()) {
                return builder()->CreateFSub(eval(e.arg(0)), eval(e.arg(1)));
            } else {
                return builder()->CreateSub(eval(e.arg(0)), eval(e.arg(1)));
            }
        }
        case MathOp::MUL: {
            if (e.type.dtype.is_float()) {
                return builder()->CreateFMul(eval(e.arg(0)), eval(e.arg(1)));
            } else {
                return builder()->CreateMul(eval(e.arg(0)), eval(e.arg(1)));
            }
        }
        case MathOp::DIV: {
            if (e.type.dtype.is_float()) {
                return builder()->CreateFDiv(eval(e.arg(0)), eval(e.arg(1)));
            } else if (e.type.dtype.is_signed()) {
                return builder()->CreateSDiv(eval(e.arg(0)), eval(e.arg(1)));
            } else {
                return builder()->CreateUDiv(eval(e.arg(0)), eval(e.arg(1)));
            }
        }
        case MathOp::MOD: {
            if (e.type.dtype.is_signed()) {
                return builder()->CreateSRem(eval(e.arg(0)), eval(e.arg(1)));
            } else {
                return builder()->CreateURem(eval(e.arg(0)), eval(e.arg(1)));
            }
        }
        case MathOp::NEG: {
            if (e.type.dtype.is_float()) {
                return builder()->CreateFNeg(eval(e.arg(0)));
            } else {
                return builder()->CreateNeg(eval(e.arg(0)));
            }
        }
        case MathOp::SQRT: return builder()->CreateIntrinsic(Intrinsic::sqrt, {lltype(e.arg(0))}, {eval(e.arg(0))});
        case MathOp::POW: return builder()->CreateIntrinsic(
            Intrinsic::pow, {lltype(e.arg(0))}, {eval(e.arg(0)), eval(e.arg(1))});
        case MathOp::CEIL: return builder()->CreateIntrinsic(Intrinsic::ceil, {lltype(e.arg(0))}, {eval(e.arg(0))});
        case MathOp::FLOOR: return builder()->CreateIntrinsic(Intrinsic::floor, {lltype(e.arg(0))}, {eval(e.arg(0))});
        case MathOp::EQ: {
            if (e.arg(0)->type.dtype.is_float()) {
                return builder()->CreateFCmpOEQ(eval(e.arg(0)), eval(e.arg(1)));
            } else {
                return builder()->CreateICmpEQ(eval(e.arg(0)), eval(e.arg(1)));
            }
        }
        case MathOp::LT: {
            if (e.arg(0)->type.dtype.is_float()) {
                return builder()->CreateFCmpOLT(eval(e.arg(0)), eval(e.arg(1)));
            } else if (e.arg(0)->type.dtype.is_signed()) {
                return builder()->CreateICmpSLT(eval(e.arg(0)), eval(e.arg(1)));
            } else {
                return builder()->CreateICmpULT(eval(e.arg(0)), eval(e.arg(1)));
            }
        }
        case MathOp::LTE: {
            if (e.arg(0)->type.dtype.is_float()) {
                return builder()->CreateFCmpOLE(eval(e.arg(0)), eval(e.arg(1)));
            } else if (e.arg(0)->type.dtype.is_signed()) {
                return builder()->CreateICmpSLE(eval(e.arg(0)), eval(e.arg(1)));
            } else {
                return builder()->CreateICmpULE(eval(e.arg(0)), eval(e.arg(1)));
            }
        }
        case MathOp::GT: {
            if (e.arg(0)->type.dtype.is_float()) {
                return builder()->CreateFCmpOGT(eval(e.arg(0)), eval(e.arg(1)));
            } else if (e.arg(0)->type.dtype.is_signed()) {
                return builder()->CreateICmpSGT(eval(e.arg(0)), eval(e.arg(1)));
            } else {
                return builder()->CreateICmpUGT(eval(e.arg(0)), eval(e.arg(1)));
            }
        }
        case MathOp::GTE: {
            if (e.arg(0)->type.dtype.is_float()) {
                return builder()->CreateFCmpOGE(eval(e.arg(0)), eval(e.arg(1)));
            } else if (e.arg(0)->type.dtype.is_signed()) {
                return builder()->CreateICmpSGE(eval(e.arg(0)), eval(e.arg(1)));
            } else {
                return builder()->CreateICmpUGE(eval(e.arg(0)), eval(e.arg(1)));
            }
        }
        case MathOp::NOT: return builder()->CreateNot(eval(e.arg(0)));
        case MathOp::AND: return builder()->CreateAnd(eval(e.arg(0)), eval(e.arg(1)));
        case MathOp::OR: return builder()->CreateOr(eval(e.arg(0)), eval(e.arg(1)));
        default: throw std::runtime_error("Invalid math operation"); break;
    }
}

Value* LLVMGen::visit(const Exists& exists)
{
    return builder()->CreateIsNotNull(eval(exists.expr));
}

Value* LLVMGen::visit(const Fetch& fetch)
{
    auto& dtype = fetch.reg->type.dtype;
    auto reg_val = eval(fetch.reg);
    auto time_val = eval(fetch.time);
    auto idx_val = eval(fetch.idx);
    auto size_val = llsizeof(lltype(dtype));
    auto ret_type = lltype(types::CHAR_PTR);
    auto addr = llcall("fetch", ret_type, { reg_val, time_val, idx_val, size_val });

    return builder()->CreateBitCast(addr, lltype(fetch));
}

Value* LLVMGen::visit(const Advance& adv)
{
    return llcall("advance", lltype(adv), { adv.reg, adv.idx, adv.time });
}

Value* LLVMGen::visit(const GetCkpt& next)
{
    return llcall("get_ckpt", lltype(next), { next.reg, next.time, next.idx });
}

Value* LLVMGen::visit(const GetStartIdx& start_idx)
{
    return llcall("get_start_idx", lltype(start_idx), { start_idx.reg });
}

Value* LLVMGen::visit(const GetEndIdx& end_idx)
{
    return llcall("get_end_idx", lltype(end_idx), { end_idx.reg });
}

Value* LLVMGen::visit(const CommitNull& commit)
{
    return llcall("commit_null", lltype(commit), { commit.reg, commit.time });
}

Value* LLVMGen::visit(const CommitData& commit)
{
    return llcall("commit_data", lltype(commit), { commit.reg, commit.time });
}

Value* LLVMGen::visit(const Read& read)
{
    auto ptr_val = eval(read.ptr);
    return builder()->CreateLoad(ptr_val);
}

Value* LLVMGen::visit(const Write& write)
{
    auto reg_val = eval(write.reg);
    auto ptr_val = eval(write.ptr);
    auto data_val = eval(write.data);
    builder()->CreateStore(data_val, ptr_val);
    return reg_val;
}

Value* LLVMGen::visit(const AllocRegion& alloc)
{
    auto time_val = eval(alloc.start_time);
    auto size_val = eval(alloc.size);
    auto tl_arr = builder()->CreateAlloca(lltype(types::IVAL), size_val);
    auto data_arr = builder()->CreateAlloca(lltype(alloc.type.dtype), size_val);
    auto char_arr = builder()->CreateBitCast(data_arr, lltype(types::CHAR_PTR));

    auto reg_type = lltype(alloc);
    auto reg_val = builder()->CreateAlloca(reg_type->getPointerElementType());
    return llcall("init_region", lltype(alloc), { reg_val, time_val, tl_arr, char_arr });
}

Value* LLVMGen::visit(const MakeRegion& make_reg)
{
    auto in_reg_val = eval(make_reg.reg);
    auto st_val = eval(make_reg.st);
    auto si_val = eval(make_reg.si);
    auto et_val = eval(make_reg.et);
    auto ei_val = eval(make_reg.ei);
    auto reg_type = lltype(make_reg);
    auto out_reg_val = builder()->CreateAlloca(reg_type->getPointerElementType());
    return llcall("make_region", reg_type, { out_reg_val, in_reg_val, st_val, si_val, et_val, ei_val });
}

Value* LLVMGen::visit(const Call& call)
{
    return llcall(call.name, lltype(call), call.args);
}

Value* LLVMGen::visit(const Loop& loop)
{
    // Build inner loops
    for (const auto& inner_loop : loop.inner_loops) {
        LLVMGenCtx new_ctx(inner_loop.get(), &llctx());
        auto& old_ctx = switch_ctx(new_ctx);
        inner_loop->Accept(*this);
        switch_ctx(old_ctx);
    }

    // Build current loop
    auto preheader_bb = BasicBlock::Create(llctx(), "preheader");
    auto header_bb = BasicBlock::Create(llctx(), "header");
    auto body_bb = BasicBlock::Create(llctx(), "body");
    auto end_bb = BasicBlock::Create(llctx(), "end");
    auto exit_bb = BasicBlock::Create(llctx(), "exit");

    // Define function signature
    vector<llvm::Type*> args_type;
    for (const auto& input : loop.inputs) {
        args_type.push_back(lltype(input->type));
    }
    auto loop_fn = llfunc(loop.get_name(), lltype(loop.output), args_type);
    for (size_t i = 0; i < loop.inputs.size(); i++) {
        auto input = loop.inputs[i];
        assign(input, loop_fn->getArg(i));
    }

    // Initialization of loop states
    loop_fn->getBasicBlockList().push_back(preheader_bb);
    builder()->SetInsertPoint(preheader_bb);
    map<Sym, llvm::Value*> base_inits;
    for (const auto& [_, base] : loop.state_bases) {
        base_inits[base] = eval(loop.syms.at(base));
    }
    builder()->CreateBr(header_bb);

    // Phi nodes for loop states
    loop_fn->getBasicBlockList().push_back(header_bb);
    builder()->SetInsertPoint(header_bb);
    for (const auto& [base_sym, val] : base_inits) {
        auto base = builder()->CreatePHI(lltype(base_sym->type), 2, base_sym->name);
        assign(base_sym, base);
        base->addIncoming(val, preheader_bb);
    }

    // Check exit condition
    builder()->CreateCondBr(eval(loop.exit_cond), exit_bb, body_bb);

    // Loop body
    loop_fn->getBasicBlockList().push_back(body_bb);
    builder()->SetInsertPoint(body_bb);
    auto stack_val = builder()->CreateIntrinsic(Intrinsic::stacksave, {}, {});

    // Update loop counter
    eval(loop.t);

    // Update indices
    for (const auto& idx : loop.idxs) {
        eval(idx);
    }

    // Evaluate loop output
    eval(loop.output);
    for (const auto& [var, base] : loop.state_bases) {
        auto base_phi = dyn_cast<PHINode>(eval(base));
        base_phi->addIncoming(eval(var), end_bb);
    }
    builder()->CreateBr(end_bb);

    // Jump back to loop header
    loop_fn->getBasicBlockList().push_back(end_bb);
    builder()->SetInsertPoint(end_bb);
    builder()->CreateIntrinsic(Intrinsic::stackrestore, {}, {stack_val});
    builder()->CreateBr(header_bb);

    // Loop exit
    loop_fn->getBasicBlockList().push_back(exit_bb);
    builder()->SetInsertPoint(exit_bb);
    builder()->CreateRet(eval(loop.state_bases.at(loop.output)));

    return loop_fn;
}

unique_ptr<llvm::Module> LLVMGen::Build(const Looper loop, llvm::LLVMContext& llctx)
{
    LLVMGenCtx ctx(loop.get(), &llctx);
    LLVMGen llgen(move(ctx));
    loop->Accept(llgen);
    return move(llgen._llmod);
}
