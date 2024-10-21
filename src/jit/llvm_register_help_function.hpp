#ifndef LLVM_REGISTER_HELP_FUNCTION_HPP
#define LLVM_REGISTER_HELP_FUNCTION_HPP
#include "../config.hpp"
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>


namespace RexVM {

    struct LLVMHelpFunction {

        explicit LLVMHelpFunction(llvm::Module &module);

        llvm::FunctionCallee getInstanceConstant{};
        llvm::FunctionCallee returnCommon{};
        llvm::FunctionCallee invokeMethodFixed{};
        llvm::FunctionCallee newObject{};
        llvm::FunctionCallee throwException{};
        llvm::FunctionCallee matchCatch{};
        llvm::FunctionCallee misc{};

        llvm::Value *createCallGetInstanceConstant(llvm::IRBuilder<> &irBuilder, llvm::Value *framePtr, u2 index) const;

        void createCallReturnCommon(llvm::IRBuilder<> &irBuilder, llvm::Value *framePtr, llvm::Value *value, u1 type) const;

        llvm::Value *createCallInvokeMethodFixed(llvm::IRBuilder<> &irBuilder, llvm::Value *framePtr, llvm::Value *method, u2 paramSlotSize, u4 pc, cview methodName, bool isClinit) const;

        llvm::Value *createCallNew(llvm::IRBuilder<> &irBuilder, llvm::Value *framePtr, uint8_t type, llvm::Value *length, llvm::Value *klass, llvm::Value *hasException) const;

        void createCallThrowException(llvm::IRBuilder<> &irBuilder, llvm::Value *framePtr, llvm::Value *exception, u4 pc, u1 fixedException, llvm::Value *exField) const;

        llvm::Value *createCallMatchCatch(llvm::IRBuilder<> &irBuilder, llvm::Value *oop, llvm::Value *catchClasses, u4 size) const;

        llvm::Value * createCallMisc(llvm::IRBuilder<> &irBuilder, llvm::Value *framePtr, llvm::Value *pa, llvm::Value *pb, u1 type) const;

    };

}

#endif
