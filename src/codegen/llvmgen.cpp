#include "tilt/base/type.h"
#include "tilt/codegen/llvmgen.h"
#include "tilt/codegen/vinstr.h"

#include "llvm/IR/Function.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/Linker/Linker.h"

#include "easy/jit.h"

using namespace std;
using namespace std::placeholders;
using namespace tilt;
using namespace llvm;

void LLVMGen::register_vinstrs()
{
    Linker::linkModules(*llmod(), easy::get_module(llctx(), get_start_idx, _1));
    Linker::linkModules(*llmod(), easy::get_module(llctx(), get_end_idx, _1));
    Linker::linkModules(*llmod(), easy::get_module(llctx(), get_time, _1));
    Linker::linkModules(*llmod(), easy::get_module(llctx(), get_index, _1));
    Linker::linkModules(*llmod(), easy::get_module(llctx(), next_time, _1, _2));
    Linker::linkModules(*llmod(), easy::get_module(llctx(), advance, _1, _2, _3));
    Linker::linkModules(*llmod(), easy::get_module(llctx(), fetch, _1, _2, _3));
    Linker::linkModules(*llmod(), easy::get_module(llctx(), make_region, _1, _2, _3, _4));
    Linker::linkModules(*llmod(), easy::get_module(llctx(), alloc_region, _1, _2, _3, _4));
    Linker::linkModules(*llmod(), easy::get_module(llctx(), commit_data, _1, _2));
    Linker::linkModules(*llmod(), easy::get_module(llctx(), commit_null, _1, _2));
}

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
    for (const auto& arg_val: arg_vals) {
        arg_types.push_back(arg_val->getType());
    }
    auto fn = llfunc(name, ret_type, arg_types);
    return builder()->CreateCall(fn, arg_vals);
}

Value* LLVMGen::llcall(const string name, llvm::Type* ret_type, vector<ExprPtr> args)
{
    vector<Value*> arg_vals;
    for (const auto& arg: args) {
        arg_vals.push_back(eval(arg));
    }

    return llcall(name, ret_type, arg_vals);
}

Value* LLVMGen::llsizeof(llvm::Type* type)
{
    auto size = llmod()->getDataLayout().getTypeSizeInBits(type).getFixedSize();
    return ConstantInt::get(lltype(types::UINT32), size/8);
}

llvm::Type* LLVMGen::lltype(const PrimitiveType& btype)
{
    switch (btype)
    {
    case PrimitiveType::BOOL:
        return llvm::Type::getInt1Ty(llctx());
    case PrimitiveType::CHAR:
        return llvm::Type::getInt8Ty(llctx());
    case PrimitiveType::INT8:
    case PrimitiveType::UINT8:
        return llvm::Type::getInt8Ty(llctx());
    case PrimitiveType::INT16:
    case PrimitiveType::UINT16:
        return llvm::Type::getInt16Ty(llctx());
    case PrimitiveType::INT32:
    case PrimitiveType::UINT32:
        return llvm::Type::getInt32Ty(llctx());
    case PrimitiveType::INT64:
    case PrimitiveType::UINT64:
        return llvm::Type::getInt64Ty(llctx());
    case PrimitiveType::FLOAT32:
        return llvm::Type::getFloatTy(llctx());
    case PrimitiveType::FLOAT64:
        return llvm::Type::getDoubleTy(llctx());
    case PrimitiveType::TIMESTAMP:
        return llvm::Type::getInt64Ty(llctx());
    case PrimitiveType::TIME:
        return llvm::Type::getInt64Ty(llctx());
    case PrimitiveType::INDEX:
        return llmod()->getTypeByName("struct.index_t");
    case PrimitiveType::UNKNOWN:
    default:
        throw std::runtime_error("Invalid type");
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
        type = StructType::get(llctx(), lltypes);
    } else {
        type = lltypes[0];
    }

    if (is_ptr) {
        type = PointerType::get(type, 0);
    }

    return type;
}

llvm::Type* LLVMGen::lltype(const Type& type)
{
    if (type.isLStream()) {
        auto reg_type = llmod()->getTypeByName("struct.region_t");
        return PointerType::get(reg_type, 0);
    } else {
        return lltype(type.dtype);
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

Value* LLVMGen::visit(const IConst& iconst)
{
    return ConstantInt::getSigned(lltype(iconst), iconst.val);
}

Value* LLVMGen::visit(const UConst& uconst)
{
    return ConstantInt::get(lltype(uconst), uconst.val);
}

Value* LLVMGen::visit(const FConst& fconst)
{
    return ConstantFP::get(lltype(fconst), fconst.val);
}

Value* LLVMGen::visit(const CConst& cconst)
{
    return ConstantInt::get(lltype(cconst), cconst.val);
}

Value* LLVMGen::visit(const TConst& tconst)
{
    return ConstantInt::get(lltype(tconst), tconst.val);
}

Value* LLVMGen::visit(const Add& add)
{
    return builder()->CreateAdd(eval(add.Left()), eval(add.Right()));
}

Value* LLVMGen::visit(const Sub& sub)
{
    return builder()->CreateSub(eval(sub.Left()), eval(sub.Right()));
}

Value* LLVMGen::visit(const Max& max)
{
    auto left = eval(max.Left());
    auto right = eval(max.Right());
    auto ge = builder()->CreateICmpSGE(left, right);
    return builder()->CreateSelect(ge, left, right);
}

Value* LLVMGen::visit(const Min& min)
{
    auto left = eval(min.Left());
    auto right = eval(min.Right());
    auto le = builder()->CreateICmpSLE(left, right);
    return builder()->CreateSelect(le, left, right);
}

Value* LLVMGen::visit(const Now&) { throw std::runtime_error("Invalid expression"); }

Value* LLVMGen::visit(const Exists& exists)
{
    return builder()->CreateIsNotNull(eval(exists.sym));
}

Value* LLVMGen::visit(const Equals& equals)
{
    return builder()->CreateICmpEQ(eval(equals.Left()), eval(equals.Right()));
}

Value* LLVMGen::visit(const Not& not_expr)
{
    return builder()->CreateNot(eval(not_expr.Input()));
}

Value* LLVMGen::visit(const And& and_expr)
{
    return builder()->CreateAnd(eval(and_expr.Left()), eval(and_expr.Right()));
}

Value* LLVMGen::visit(const Or& or_expr)
{
    return builder()->CreateOr(eval(or_expr.Left()), eval(or_expr.Right()));
}

Value* LLVMGen::visit(const True&)
{
    return ConstantInt::getTrue(llctx());
}

Value* LLVMGen::visit(const False&)
{
    return ConstantInt::getFalse(llctx());
}

Value* LLVMGen::visit(const LessThan& lt)
{
    return builder()->CreateICmpSLT(eval(lt.Left()), eval(lt.Right()));
}

Value* LLVMGen::visit(const LessThanEqual& lte)
{
    return builder()->CreateICmpSLE(eval(lte.Left()), eval(lte.Right()));
}

Value* LLVMGen::visit(const GreaterThan& gt)
{
    return builder()->CreateICmpSGT(eval(gt.Left()), eval(gt.Right()));
}

Value* LLVMGen::visit(const GetTime& get_time)
{
    return llcall("get_time", lltype(get_time), { eval(get_time.idx) });
}

Value* LLVMGen::visit(const GetIndex& get_idx)
{
    return llcall("get_index", lltype(get_idx), { eval(get_idx.idx) });
}

Value* LLVMGen::visit(const Fetch& fetch)
{
    auto& dtype = fetch.reg->type.dtype;
    auto reg_val = eval(fetch.reg);
    auto idx_val = eval(fetch.idx);
    auto size_val = llsizeof(lltype(dtype));
    auto ret_type = lltype({PrimitiveType::INT8}, true);
    auto addr = llcall("fetch", ret_type, { reg_val, idx_val, size_val });

    return builder()->CreateBitCast(addr, lltype(dtype.ptypes, true));
}

Value* LLVMGen::visit(const Advance& adv)
{
    return llcall("advance", lltype(adv), { adv.reg, adv.idx, adv.time });
}

Value* LLVMGen::visit(const NextTime& next)
{
    return llcall("next_time", lltype(next), { next.reg, next.idx });
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

Value* LLVMGen::visit(const AllocIndex& alloc_idx)
{
    auto init_val = builder()->CreateLoad(eval(alloc_idx.init_idx));
    auto idx_ptr = builder()->CreateAlloca(lltype(types::INDEX));
    builder()->CreateStore(init_val, idx_ptr);
    return idx_ptr;
}

Value* LLVMGen::visit(const Load& load)
{
    auto ptr_val = eval(load.ptr);
    return builder()->CreateLoad(ptr_val);
}

Value* LLVMGen::visit(const Store& store)
{
    auto reg_val = eval(store.reg);
    auto ptr_val = eval(store.ptr);
    auto data_val = eval(store.data);
    builder()->CreateStore(data_val, ptr_val);
    return reg_val;
}

Value* LLVMGen::visit(const AllocRegion& alloc)
{
    auto time_val = eval(alloc.start_time);
    auto size_val = eval(alloc.size);
    auto tl_arr = builder()->CreateAlloca(lltype(types::INDEX), size_val);
    auto data_arr = builder()->CreateAlloca(lltype(alloc.type.dtype), size_val);
    auto char_arr = builder()->CreateBitCast(data_arr, lltype(types::CHAR.ptypes, true));

    auto reg_type = lltype(alloc);
    auto reg_val = builder()->CreateAlloca(reg_type->getPointerElementType());
    return llcall("alloc_region", lltype(alloc), { reg_val, time_val, tl_arr, char_arr });
}

Value* LLVMGen::visit(const MakeRegion& make_reg)
{
    auto in_reg_val = eval(make_reg.reg);
    auto start_idx_val = eval(make_reg.start_idx);
    auto end_idx_val = eval(make_reg.end_idx);
    auto reg_type = lltype(make_reg);
    auto out_reg_val = builder()->CreateAlloca(reg_type->getPointerElementType());
    return llcall("make_region", reg_type, { out_reg_val, in_reg_val, start_idx_val, end_idx_val });
}

Value* LLVMGen::visit(const Call& call)
{
    return llcall(call.fn->GetName(), lltype(call), call.args);
}

Value* LLVMGen::visit(const Loop& loop)
{
    // Build inner loops
    for (const auto& inner_loop: loop.inner_loops) {
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
    for (const auto& input: loop.inputs) {
        args_type.push_back(lltype(input->type));
    }
    auto loop_fn = llfunc(loop.GetName(), lltype(loop.output), args_type);
    for (size_t i = 0; i < loop.inputs.size(); i++) {
        auto input = loop.inputs[i];
        assign(input, loop_fn->getArg(i));
    }

    // Initialization of loop states
    loop_fn->getBasicBlockList().push_back(preheader_bb);
    builder()->SetInsertPoint(preheader_bb);
    map<SymPtr, llvm::Value*> base_inits;
    for (const auto& [_, base]: loop.state_bases) {
        base_inits[base] = eval(loop.syms.at(base));
    }
    builder()->CreateBr(header_bb);

    // Phi nodes for loop states
    loop_fn->getBasicBlockList().push_back(header_bb);
    builder()->SetInsertPoint(header_bb);
    for (const auto& [base_sym, val]: base_inits) {
        auto base = builder()->CreatePHI(lltype(base_sym->type), 2, base_sym->name);
        assign(base_sym, base);
        base->addIncoming(val, preheader_bb);
    }

    // Update loop counter and check exit condition
    eval(loop.t);
    auto exit_cond = eval(loop.exit_cond);
    builder()->CreateCondBr(exit_cond, exit_bb, body_bb);

    // Update indices
    loop_fn->getBasicBlockList().push_back(body_bb);
    builder()->SetInsertPoint(body_bb);
    auto stack_val = builder()->CreateIntrinsic(Intrinsic::stacksave, {}, {});
    for (const auto& idx: loop.idxs) {
        eval(idx);
    }

    // Loop body
    eval(loop.output);
    for (const auto& [var, base]: loop.state_bases) {
        auto base_phi = dyn_cast<PHINode>(eval(base));
        base_phi->addIncoming(eval(var), end_bb);
    }
    builder()->CreateBr(end_bb);

    // Update loop states and jump back to loop header
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