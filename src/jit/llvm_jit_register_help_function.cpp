#include "llvm_jit_register_help_function.hpp"
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>

namespace RexVM {
    using namespace llvm;

    LLVMHelpFunction::LLVMHelpFunction(llvm::Module &module) {
        auto &context = module.getContext();
        const auto voidTy = Type::getVoidTy(context);
        const auto int8Ty = Type::getInt8Ty(context);
        const auto int32Ty = Type::getInt32Ty(context);
        const auto int64Ty = Type::getInt64Ty(context);
        const auto floatTy = Type::getFloatTy(context);
        const auto doubleTy = Type::getDoubleTy(context);
        const auto voidPtrTy = PointerType::getUnqual(voidTy);

        //void (void *)
        const auto type0 = FunctionType::get(voidTy, {voidPtrTy}, false);
        //void *(void *, uint32_t);
        const auto type1 = FunctionType::get(voidPtrTy, {voidPtrTy, int32Ty}, false);

        getStringConstantHelper = module.getOrInsertFunction("llvm_compile_get_string_constant", type1);
        getClassMirrorConstantHelper = module.getOrInsertFunction("llvm_compile_get_class_mirror_constant", type1);
        throwNpeHelper = module.getOrInsertFunction("llvm_compile_throw_npe", type0);

        arrayLoadI4 = module.getOrInsertFunction("llvm_compile_array_load_i4",
                                                 FunctionType::get(int32Ty, {voidPtrTy, int8Ty, int32Ty}, false));

        arrayLoadI8 = module.getOrInsertFunction("llvm_compile_array_load_i8",
                                                 FunctionType::get(int64Ty,
                                                                   {voidPtrTy, int32Ty}, false));

        arrayLoadF4 = module.getOrInsertFunction("llvm_compile_array_load_f4",
                                                 FunctionType::get(floatTy,
                                                                   {voidPtrTy, int32Ty}, false));

        arrayLoadF8 = module.getOrInsertFunction("llvm_compile_array_load_f8",
                                                 FunctionType::get(doubleTy,
                                                                   {voidPtrTy, int32Ty}, false));

        arrayLoadObj = module.getOrInsertFunction("llvm_compile_array_load_obj",
                                                  FunctionType::get(voidPtrTy, {voidPtrTy, int32Ty},
                                                                    false));

        arrayStoreI4 = module.getOrInsertFunction("llvm_compile_array_store_i4",
                                                  FunctionType::get(voidTy, {voidPtrTy, int8Ty, int32Ty, int32Ty},
                                                                    false));

        arrayStoreI8 = module.getOrInsertFunction("llvm_compile_array_store_i8",
                                                  FunctionType::get(voidTy, {voidPtrTy, int32Ty, int64Ty}, false));

        arrayStoreF4 = module.getOrInsertFunction("llvm_compile_array_store_f4",
                                                  FunctionType::get(voidTy, {voidPtrTy, int32Ty, floatTy}, false));

        arrayStoreF8 = module.getOrInsertFunction("llvm_compile_array_store_f8",
                                                  FunctionType::get(voidTy, {voidPtrTy, int32Ty, doubleTy}, false));

        arrayStoreObj = module.getOrInsertFunction("llvm_compile_array_store_obj",
                                                   FunctionType::get(voidTy, {voidPtrTy, int32Ty, voidPtrTy}, false));

        returnI4 = module.getOrInsertFunction("llvm_compile_return_i4",
                                              FunctionType::get(voidTy, { voidPtrTy, int32Ty}, false));

        returnI8 = module.getOrInsertFunction("llvm_compile_return_i8",
                                              FunctionType::get(voidTy, {voidPtrTy, int64Ty}, false));

        returnF4 = module.getOrInsertFunction("llvm_compile_return_f4",
                                              FunctionType::get(voidTy, {voidPtrTy, floatTy}, false));

        returnF8 = module.getOrInsertFunction("llvm_compile_return_f8",
                                              FunctionType::get(voidTy, {voidPtrTy, doubleTy}, false));

        returnObj = module.getOrInsertFunction("llvm_compile_return_obj",
                                               FunctionType::get(voidTy, {voidPtrTy, voidPtrTy}, false));

        returnVoid = module.getOrInsertFunction("llvm_compile_return_void",
                                                FunctionType::get(voidTy, {voidPtrTy}, false));
    }

    void LLVMHelpFunction::createCallThrowNPE(IRBuilder<> &irBuilder, Value *framePtr) const {
        irBuilder.CreateCall(throwNpeHelper, {framePtr});
    }

    Value *LLVMHelpFunction::createCallGetStringConstant(IRBuilder<> &irBuilder, Value *framePtr, u2 index) const {
        return irBuilder.CreateCall(getStringConstantHelper, {framePtr, irBuilder.getInt32(index)});
    }

    Value *LLVMHelpFunction::createCallGetClassMirrorConstant(IRBuilder<> &irBuilder, Value *framePtr, u2 index) const {
        return irBuilder.CreateCall(getClassMirrorConstantHelper, {framePtr, irBuilder.getInt32(index)});
    }

    Value *LLVMHelpFunction::createCallArrayLoadI4(IRBuilder<> &irBuilder, Value *arrayRef, Value *index, uint8_t type) const {
        return irBuilder.CreateCall(arrayLoadI4, {arrayRef, irBuilder.getInt8(type), index});
    }

    Value *LLVMHelpFunction::createCallArrayLoadI8(IRBuilder<> &irBuilder, Value *arrayRef, Value *index) const {
        return irBuilder.CreateCall(arrayLoadI8, {arrayRef, index});
    }

    Value *LLVMHelpFunction::createCallArrayLoadF4(IRBuilder<> &irBuilder, Value *arrayRef, Value *index) const {
        return irBuilder.CreateCall(arrayLoadF4, {arrayRef, index});
    }

    Value *LLVMHelpFunction::createCallArrayLoadF8(IRBuilder<> &irBuilder, Value *arrayRef, Value *index) const {
        return irBuilder.CreateCall(arrayLoadF8, {arrayRef, index});
    }

    Value *LLVMHelpFunction::createCallArrayLoadObj(IRBuilder<> &irBuilder, Value *arrayRef, Value *index) const {
        return irBuilder.CreateCall(arrayLoadObj, {arrayRef, index});
    }
}
