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
        const auto ptrTy = PointerType::getUnqual(voidTy);

        const auto getInstanceConstantType = FunctionType::get(ptrTy, {ptrTy, int32Ty}, false);
        getInstanceConstant = module.getOrInsertFunction("llvm_compile_get_instance_constant", getInstanceConstantType);

        const auto arrayLengthType = FunctionType::get(int32Ty, {ptrTy}, false);
        arrayLength = module.getOrInsertFunction("llvm_compile_array_length", arrayLengthType);

        const auto returnCommonType = FunctionType::get(voidTy, {ptrTy, int64Ty, int8Ty}, false);
        returnCommon = module.getOrInsertFunction("llvm_compile_return_common", returnCommonType);

        const auto clinitType = FunctionType::get(voidTy, {ptrTy, ptrTy}, false);
        clinit = module.getOrInsertFunction("llvm_compile_clinit", clinitType);

        const auto invokeMethodType = FunctionType::get(ptrTy, {ptrTy, ptrTy, int16Ty, int32Ty}, false);
        invokeMethodFixed = module.getOrInsertFunction("llvm_compile_invoke_method_fixed", invokeMethodType);

        const auto newObjectType = FunctionType::get(ptrTy, {ptrTy, int8Ty, int32Ty, ptrTy}, false);
        newObject = module.getOrInsertFunction("llvm_compile_new_object", newObjectType);

        const auto throwType = FunctionType::get(voidTy, {ptrTy, ptrTy, int32Ty, int8Ty}, false);
        throwException = module.getOrInsertFunction("llvm_compile_throw_exception", throwType);

        const auto instanceOfType = FunctionType::get(int32Ty, {ptrTy, int8Ty, ptrTy, ptrTy, int32Ty}, false);
        instanceOf = module.getOrInsertFunction("llvm_compile_check_cast", instanceOfType);

        const auto monitorType = FunctionType::get(voidTy, {ptrTy, int8Ty}, false);
        monitor = module.getOrInsertFunction("llvm_compile_monitor", monitorType);

        const auto matchCatchType = FunctionType::get(int8Ty, {ptrTy, ptrTy}, false);
        matchCatch = module.getOrInsertFunction("llvm_compile_match_catch", matchCatchType);
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

    Value *
    LLVMHelpFunction::createCallMatchCatch(IRBuilder<> &irBuilder, Value *exClass, Value *catchClass) const {
        return irBuilder.CreateCall(monitor, {exClass, catchClass});
    }

}
