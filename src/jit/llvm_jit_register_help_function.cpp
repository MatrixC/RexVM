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
        const auto voidPtrTy = PointerType::getUnqual(voidTy);

        const auto type1 = FunctionType::get(voidPtrTy, {voidPtrTy, int32Ty}, false);

        getInstanceConstant = module.getOrInsertFunction("llvm_compile_get_instance_constant", type1);

        arrayLength = module.getOrInsertFunction("llvm_compile_array_length",
                                                 FunctionType::get(int32Ty, {voidPtrTy}, false));

        returnCommon = module.getOrInsertFunction("llvm_compile_return_common",
                                                  FunctionType::get(voidTy, {voidPtrTy, int64Ty, int8Ty}, false));

        clinit = module.getOrInsertFunction("llvm_compile_clinit",
                                            FunctionType::get(voidTy, {voidPtrTy, voidPtrTy}, false));

        invokeMethodFixed = module.getOrInsertFunction("llvm_compile_invoke_method_fixed",
                                                        FunctionType::get(
                                                            voidPtrTy, {voidPtrTy, voidPtrTy, int16Ty, int32Ty}, false));

        newObject = module.getOrInsertFunction("llvm_compile_new_object",
                                               FunctionType::get(voidPtrTy, {
                                                                     voidPtrTy, int8Ty, int32Ty, voidPtrTy
                                                                 }, false));

        throwException = module.getOrInsertFunction("llvm_compile_throw_exception",
                                                    FunctionType::get(voidTy, {voidPtrTy, voidPtrTy, int32Ty, int8Ty}, false));

        instanceOf = module.getOrInsertFunction("llvm_compile_check_cast",
                                                FunctionType::get(int32Ty, {voidPtrTy, int8Ty, voidPtrTy, voidPtrTy, int32Ty},
                                                                  false));

        monitor = module.getOrInsertFunction("llvm_compile_monitor",
                                             FunctionType::get(voidTy, {voidPtrTy, int8Ty},
                                                               false));
    }


    Value *LLVMHelpFunction::createCallGetInstanceConstant(IRBuilder<> &irBuilder, Value *framePtr,
                                                         const u2 index) const {
        return irBuilder.CreateCall(getInstanceConstant, {framePtr, irBuilder.getInt32(index)});
    }

    Value *LLVMHelpFunction::createCallArrayLength(IRBuilder<> &irBuilder, Value *arrayRef) const {
        return irBuilder.CreateCall(arrayLength, {arrayRef});
    }

    void LLVMHelpFunction::createCallReturnCommon(IRBuilder<> &irBuilder, Value *framePtr, Value *value, const u1 type) const {
        irBuilder.CreateCall(returnCommon, {framePtr, value, irBuilder.getInt8(type)});
    }

    void LLVMHelpFunction::createCallClinit(IRBuilder<> &irBuilder, Value *framePtr, Value *klass) const {
        irBuilder.CreateCall(clinit, {framePtr, klass});
    }

    Value *LLVMHelpFunction::createCallInvokeMethodFixed(IRBuilder<> &irBuilder, Value *framePtr, Value *method,
                                                        const u2 paramSlotSize, u4 pc, const cview methodName) const {
        return irBuilder.CreateCall(invokeMethodFixed, {framePtr, method, irBuilder.getInt16(paramSlotSize), irBuilder.getInt32(pc)}, methodName);
    }

    Value *LLVMHelpFunction::createCallNew(IRBuilder<> &irBuilder, Value *framePtr, const uint8_t type, Value *length,
                                           Value *klass) const {
        return irBuilder.CreateCall(newObject, {framePtr, irBuilder.getInt8(type), length, klass});
    }

    void LLVMHelpFunction::createCallThrowException(IRBuilder<> &irBuilder, Value *framePtr, Value *exception, const u4 pc, const u1 fixedException) const {
        irBuilder.CreateCall(throwException, {framePtr, exception, irBuilder.getInt32(pc), irBuilder.getInt8(fixedException)});
    }

    Value *LLVMHelpFunction::createCallInstanceOf(IRBuilder<> &irBuilder, Value *framePtr, const u1 type, Value *oop,
                                                  Value *checkClass, const u4 pc) const {
        return irBuilder.CreateCall(instanceOf, {framePtr, irBuilder.getInt8(type), oop, checkClass, irBuilder.getInt32(pc)});
    }

    void LLVMHelpFunction::createCallMonitor(IRBuilder<> &irBuilder, Value *oop, const u1 type) const {
        irBuilder.CreateCall(monitor, {oop, irBuilder.getInt8(type)});
    }
}
