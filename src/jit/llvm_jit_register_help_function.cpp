#include "llvm_jit_register_help_function.hpp"
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>

namespace RexVM {
    using namespace llvm;

    LLVMHelpFunction::LLVMHelpFunction(Module &module) {
        auto &context = module.getContext();
        const auto voidTy = Type::getVoidTy(context);
        const auto int8Ty = Type::getInt8Ty(context);
        const auto int16Ty = Type::getInt16Ty(context);
        const auto int32Ty = Type::getInt32Ty(context);
        const auto int64Ty = Type::getInt64Ty(context);
        const auto floatTy = Type::getFloatTy(context);
        const auto doubleTy = Type::getDoubleTy(context);
        const auto voidPtrTy = PointerType::getUnqual(voidTy);

        //void (void *)
        const auto type0 = FunctionType::get(voidTy, {voidPtrTy, int32Ty}, false);
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

        clinit = module.getOrInsertFunction("llvm_compile_clinit",
                                                FunctionType::get(voidTy, {voidPtrTy, voidPtrTy}, false));

        getField = module.getOrInsertFunction("llvm_compile_get_field",
                                              FunctionType::get(voidPtrTy, {voidPtrTy, int16Ty}, false));

        invokeMethod = module.getOrInsertFunction("llvm_compile_invoke_method",
                                                  FunctionType::get(voidTy, {voidPtrTy, int16Ty, int16Ty, int8Ty}, false));

    }

    void LLVMHelpFunction::createCallThrowNPE(IRBuilder<> &irBuilder, Value *framePtr, const u4 pc) const {
        irBuilder.CreateCall(throwNpeHelper, {framePtr, irBuilder.getInt32(pc)});
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

    void LLVMHelpFunction::createCallArrayStoreI4(IRBuilder<> &irBuilder, Value *arrayRef, Value *index, Value *value, const uint8_t type) const {
        irBuilder.CreateCall(arrayStoreI4, {arrayRef, irBuilder.getInt8(type), index, value}); 
    }

    void LLVMHelpFunction::createCallArrayStoreI8(IRBuilder<> &irBuilder, Value *arrayRef, Value *index, Value *value) const {
        irBuilder.CreateCall(arrayStoreI8, {arrayRef, index, value}); 
    }

    void LLVMHelpFunction::createCallArrayStoreF4(IRBuilder<> &irBuilder, Value *arrayRef, Value *index, Value *value) const {
        irBuilder.CreateCall(arrayStoreF4, {arrayRef, index, value}); 
    }

    void LLVMHelpFunction::createCallArrayStoreF8(IRBuilder<> &irBuilder, Value *arrayRef, Value *index, Value *value) const {
        irBuilder.CreateCall(arrayStoreF8, {arrayRef, index, value}); 
    }

    void LLVMHelpFunction::createCallArrayStoreObj(IRBuilder<> &irBuilder, Value *arrayRef, Value *index, Value *value) const {
        irBuilder.CreateCall(arrayStoreObj, {arrayRef, index, value}); 
    }

    void LLVMHelpFunction::createCallReturnI4(IRBuilder<> &irBuilder, Value *framePtr, Value *value) const {
        irBuilder.CreateCall(returnI4, {framePtr, value});
    }

    void LLVMHelpFunction::createCallReturnI8(IRBuilder<> &irBuilder, Value *framePtr, Value *value) const {
        irBuilder.CreateCall(returnI8, {framePtr, value});
    }

    void LLVMHelpFunction::createCallReturnF4(IRBuilder<> &irBuilder, Value *framePtr, Value *value) const {
        irBuilder.CreateCall(returnF4, {framePtr, value});
    }

    void LLVMHelpFunction::createCallReturnF8(IRBuilder<> &irBuilder, Value *framePtr, Value *value) const {
        irBuilder.CreateCall(returnF8, {framePtr, value});
    }

    void LLVMHelpFunction::createCallReturnObj(IRBuilder<> &irBuilder, Value *framePtr, Value *value) const {
        irBuilder.CreateCall(returnObj, {framePtr, value});
    }

    void LLVMHelpFunction::createCallReturnVoid(IRBuilder<> &irBuilder, Value *framePtr) const {
        irBuilder.CreateCall(returnVoid, {framePtr});
    }

    void LLVMHelpFunction::createCallClinit(IRBuilder<> &irBuilder, Value *framePtr, Value *klass) const {
        irBuilder.CreateCall(clinit, {framePtr, klass});
    }

    Value *LLVMHelpFunction::createCallGetFieldPtr(IRBuilder<> &irBuilder, Value *klass, Value *index) const {
        return irBuilder.CreateCall(getField, {klass, index});
    }

    void LLVMHelpFunction::createCallInvokeMethod(IRBuilder<> &irBuilder, Value *framePtr, const uint16_t index,
                                                  const uint16_t paramSlotSize, const uint8_t invokeType) const {
        irBuilder.CreateCall(invokeMethod, {
                                 framePtr, irBuilder.getInt16(index), irBuilder.getInt16(paramSlotSize),
                                 irBuilder.getInt8(invokeType)
                             });
    }
}
