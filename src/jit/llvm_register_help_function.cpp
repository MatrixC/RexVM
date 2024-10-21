#include "llvm_register_help_function.hpp"
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

        const auto returnCommonType = FunctionType::get(voidTy, {ptrTy, int64Ty, int8Ty}, false);
        returnCommon = module.getOrInsertFunction("llvm_compile_return_common", returnCommonType);

        const auto invokeMethodType = FunctionType::get(ptrTy, {ptrTy, ptrTy, int16Ty, int32Ty, int8Ty}, false);
        invokeMethodFixed = module.getOrInsertFunction("llvm_compile_invoke_method_fixed", invokeMethodType);

        const auto newObjectType = FunctionType::get(ptrTy, {ptrTy, int8Ty, int32Ty, ptrTy, ptrTy}, false);
        newObject = module.getOrInsertFunction("llvm_compile_new_object", newObjectType);

        const auto throwType = FunctionType::get(voidTy, {ptrTy, ptrTy, int32Ty, int8Ty, ptrTy}, false);
        throwException = module.getOrInsertFunction("llvm_compile_throw_exception", throwType);

        const auto matchCatchType = FunctionType::get(int32Ty, {ptrTy, ptrTy, int32Ty}, false);
        matchCatch = module.getOrInsertFunction("llvm_compile_match_catch", matchCatchType);

        const auto miscType = FunctionType::get(int32Ty, {ptrTy, ptrTy, ptrTy, int8Ty}, false);
        misc = module.getOrInsertFunction("llvm_compile_misc", miscType);
    }


    Value *LLVMHelpFunction::createCallGetInstanceConstant(IRBuilder<> &irBuilder, Value *framePtr,
                                                         const u2 index) const {
        return irBuilder.CreateCall(getInstanceConstant, {framePtr, irBuilder.getInt32(index)});
    }

    void LLVMHelpFunction::createCallReturnCommon(IRBuilder<> &irBuilder, Value *framePtr, Value *value, const u1 type) const {
        irBuilder.CreateCall(returnCommon, {framePtr, value, irBuilder.getInt8(type)});
    }

    Value *LLVMHelpFunction::createCallInvokeMethodFixed(IRBuilder<> &irBuilder, Value *framePtr, Value *method,
                                                        const u2 paramSlotSize, u4 pc, const cview methodName, const bool isClinit) const {
        return irBuilder.CreateCall(
            invokeMethodFixed, {
                framePtr,
                method,
                irBuilder.getInt16(paramSlotSize),
                irBuilder.getInt32(pc),
                irBuilder.getInt8(isClinit ? 1 : 0)
            }, methodName);
    }

    Value *LLVMHelpFunction::createCallNew(IRBuilder<> &irBuilder, Value *framePtr, const uint8_t type, Value *length,
                                           Value *klass, Value *hasException) const {
        return irBuilder.CreateCall(newObject, {framePtr, irBuilder.getInt8(type), length, klass, hasException});
    }

    void LLVMHelpFunction::createCallThrowException(
        IRBuilder<> &irBuilder, Value *framePtr, Value *exception,
        const u4 pc, const u1 fixedException, Value *exField
    ) const {
        irBuilder.CreateCall(
            throwException,
            {framePtr, exception, irBuilder.getInt32(pc), irBuilder.getInt8(fixedException), exField}
        );
    }

    Value *
    LLVMHelpFunction::createCallMatchCatch(IRBuilder<> &irBuilder, Value *oop, Value *catchClasses, const u4 size) const {
        return irBuilder.CreateCall(matchCatch, {oop, catchClasses, irBuilder.getInt32(size)});
    }

    Value *LLVMHelpFunction::createCallMisc(IRBuilder<> &irBuilder, Value *framePtr, Value *pa, Value *pb,
                                            const u1 type) const {
        return irBuilder.CreateCall(misc, {framePtr, pa, pb, irBuilder.getInt8(type)});
    }


}
