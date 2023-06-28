#include "tilt/base/type.h"
#include "tilt/pass/printer.h"
#include "tilt/pass/codegen/llvmgen.h"

#include "llvm/IR/Function.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/InstrTypes.h"

using namespace tilt;
using namespace llvm;

Function* LLVMGen::llfunc(const string name, llvm::Type* ret_type, vector<llvm::Type*> arg_types)
{
    auto fn_type = FunctionType::get(ret_type, arg_types, false);
    return Function::Create(fn_type, Function::ExternalLinkage, name, llmod());
}

Value* LLVMGen::llcall(const string name, llvm::Type* ret_type, vector<Value*> arg_vals)
{
    vector<llvm::Type*> arg_types;
    for (const auto& arg_val : arg_vals) {
        arg_types.push_back(arg_val->getType());
    }

    auto fn_type = FunctionType::get(ret_type, arg_types, false);
    auto fn = llmod()->getOrInsertFunction(name, fn_type);
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
        case BaseType::IVAL:
            return StructType::getTypeByName(llctx(), "struct.ival_t");
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
    if (type.is_val()) {
        return lltype(type.dtype);
    } else {
        return llregptrtype();
    }
}

Value* LLVMGen::visit(const Symbol& symbol) { return get_expr(get_sym(symbol)); }

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
    auto new_type = lltype(_new);
    auto ptr = builder()->CreateAlloca(new_type);

    for (size_t i = 0; i < _new.inputs.size(); i++) {
        auto val_ptr = builder()->CreateStructGEP(new_type, ptr, i);
        builder()->CreateStore(eval(_new.inputs[i]), val_ptr);
    }

    return builder()->CreateLoad(new_type, ptr);
}

Value* LLVMGen::visit(const ConstNode& cnst)
{
    switch (cnst.type.dtype.btype) {
        case BaseType::BOOL:
        case BaseType::INT8:
        case BaseType::INT16:
        case BaseType::INT32:
        case BaseType::INT64:
        case BaseType::UINT8:
        case BaseType::UINT16:
        case BaseType::UINT32:
        case BaseType::UINT64:
        case BaseType::TIME:
            return ConstantInt::get(lltype(cnst), cnst.val);
        case BaseType::FLOAT32:
        case BaseType::FLOAT64:
            return ConstantFP::get(lltype(cnst), cnst.val);
        default:
            throw std::runtime_error("Invalid constant type");
            break;
    }
}

Value* LLVMGen::visit(const Cast& e)
{
    auto input_val = eval(e.arg);
    auto dest_type = lltype(e);
    auto op = CastInst::getCastOpcode(input_val, e.arg->type.dtype.is_signed(), dest_type, e.type.dtype.is_signed());
    return builder()->CreateCast(op, input_val, dest_type);
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
        case MathOp::MAX: {
            auto left = eval(e.arg(0));
            auto right = eval(e.arg(1));

            Value* cond;
            if (e.type.dtype.is_float()) {
                cond = builder()->CreateFCmpOGE(left, right);
            } else if (e.type.dtype.is_signed()) {
                cond = builder()->CreateICmpSGE(left, right);
            } else {
                cond = builder()->CreateICmpUGE(left, right);
            }
            return builder()->CreateSelect(cond, left, right);
        }
        case MathOp::MIN: {
            auto left = eval(e.arg(0));
            auto right = eval(e.arg(1));

            Value* cond;
            if (e.type.dtype.is_float()) {
                cond = builder()->CreateFCmpOLE(left, right);
            } else if (e.type.dtype.is_signed()) {
                cond = builder()->CreateICmpSLE(left, right);
            } else {
                cond = builder()->CreateICmpULE(left, right);
            }
            return builder()->CreateSelect(cond, left, right);
        }
        case MathOp::MOD: {
            if (e.type.dtype.is_signed()) {
                return builder()->CreateSRem(eval(e.arg(0)), eval(e.arg(1)));
            } else {
                return builder()->CreateURem(eval(e.arg(0)), eval(e.arg(1)));
            }
        }
        case MathOp::ABS: {
            auto input = eval(e.arg(0));

            if (e.type.dtype.is_float()) {
                return builder()->CreateIntrinsic(Intrinsic::fabs, {lltype(e.arg(0))}, {input});
            } else {
                auto neg = builder()->CreateNeg(input);

                Value* cond;
                if (e.type.dtype.is_signed()) {
                    cond = builder()->CreateICmpSGE(input, ConstantInt::get(lltype(types::INT32), 0));
                } else {
                    cond = builder()->CreateICmpUGE(input, ConstantInt::get(lltype(types::UINT32), 0));
                }
                return builder()->CreateSelect(cond, input, neg);
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
    return builder()->CreateIsNotNull(eval(exists.sym));
}

Value* LLVMGen::visit(const Fetch& fetch)
{
    auto& type = fetch.reg->type;
    auto reg_val = eval(fetch.reg);
    auto time_val = eval(fetch.time);
    auto dur_val = ConstantInt::get(lltype(types::UINT32), type.iter.period);
    auto size_val = llsizeof(lltype(type.dtype));
    auto ret_type = lltype(types::CHAR_PTR);
    auto addr = llcall("fetch", ret_type, { reg_val, time_val, dur_val, size_val });

    return builder()->CreateBitCast(addr, lltype(fetch));
}

Value* LLVMGen::visit(const GetCkpt& next)
{
    return llcall("get_ckpt", lltype(next), { next.reg, next.time });
}

Value* LLVMGen::visit(const GetStartTime& start_time)
{
    return llcall("get_start_time", lltype(start_time), { start_time.reg });
}

Value* LLVMGen::visit(const GetEndTime& end_time)
{
    return llcall("get_end_time", lltype(end_time), { end_time.reg });
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
    auto ptr_type = read.ptr->type.dtype;
    return builder()->CreateLoad(lltype(ptr_type.deref()), ptr_val);
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
    auto size_val = llcall("get_buf_size", lltype(types::UINT32), { eval(alloc.size) });
    auto data_arr = builder()->CreateAlloca(lltype(alloc.type.dtype), size_val);
    auto char_arr = builder()->CreateBitCast(data_arr, lltype(types::CHAR_PTR));
    auto reg_val = builder()->CreateAlloca(llregtype());
    return llcall("init_region", lltype(alloc), { reg_val, time_val, size_val, char_arr });
}

Value* LLVMGen::visit(const MakeRegion& make_reg)
{
    auto in_reg_val = eval(make_reg.reg);
    auto st_val = eval(make_reg.st);
    auto et_val = eval(make_reg.et);
    auto out_reg_val = builder()->CreateAlloca(llregtype());
    return llcall("make_region", lltype(make_reg), { out_reg_val, in_reg_val, st_val, et_val });
}

Value* LLVMGen::visit(const Call& call)
{
    return llcall(call.name, lltype(call), call.args);
}

Value* LLVMGen::visit(const LoopNode& loop)
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
        set_expr(input, loop_fn->getArg(i));
    }
    // We add `noalias` attribute to the region parameters to help compiler autovectorize
    for (size_t i = 0; i < loop.inputs.size(); i++) {
        // If type is not a value, then it should be a region
        if (!loop.inputs[i]->type.is_val()) {
            loop_fn->addParamAttr(i, Attribute::NoAlias);
        }
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
        set_expr(base_sym, base);
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

unique_ptr<llvm::Module> LLVMGen::Build(const Loop loop, llvm::LLVMContext& llctx)
{
    LLVMGenCtx ctx(loop.get(), &llctx);
    LLVMGen llgen(std::move(ctx));
    loop->Accept(llgen);
    return std::move(llgen._llmod);
}

void LLVMGen::register_vinstrs() {
    const auto buffer = llvm::MemoryBuffer::getMemBuffer(llvm::StringRef(vinstr_str));

    llvm::SMDiagnostic error;
    std::unique_ptr<llvm::Module> vinstr_mod = llvm::parseIR(*buffer, error, llctx());
    if (!vinstr_mod) {
        throw std::runtime_error("Failed to parse vinstr bitcode");
    }
    if (llvm::verifyModule(*vinstr_mod)) {
        throw std::runtime_error("Failed to verify vinstr module");
    }

    // For some reason if we try to set internal linkage before we link
    // modules, then the JIT will be unable to find the symbols.
    // Instead we collect the function names first, then add internal
    // linkage to them after linking the modules
    std::vector<string> vinstr_names;
    for (const auto& function : vinstr_mod->functions()) {
        if (function.isDeclaration()) {
            continue;
        }
        vinstr_names.push_back(function.getName().str());
    }

    llvm::Linker::linkModules(*llmod(), std::move(vinstr_mod));
    for (const auto& name : vinstr_names) {
        llmod()->getFunction(name.c_str())->setLinkage(llvm::Function::InternalLinkage);
    }
}
