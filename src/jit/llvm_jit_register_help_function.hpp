#ifndef LLVM_JIT_REGISTER_HELP_FUNCTION_HPP
#define LLVM_JIT_REGISTER_HELP_FUNCTION_HPP
#include "../config.hpp"
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>


namespace RexVM {

    struct LLVMHelpFunction {
        explicit LLVMHelpFunction(llvm::Module &module);

        llvm::FunctionCallee getInstanceConstant{};
        llvm::FunctionCallee arrayLength{};
        llvm::FunctionCallee returnCommon{};
        llvm::FunctionCallee clinit{};
        llvm::FunctionCallee invokeMethodFixed{};
        llvm::FunctionCallee newObject{};
        llvm::FunctionCallee throwException{};
        llvm::FunctionCallee instanceOf{};
        llvm::FunctionCallee monitor{};


        llvm::Value *createCallGetInstanceConstant(llvm::IRBuilder<> &irBuilder, llvm::Value *framePtr, u2 index) const;

        llvm::Value *createCallArrayLength(llvm::IRBuilder<> &irBuilder, llvm::Value *arrayRef) const;
        void createCallReturnCommon(llvm::IRBuilder<> &irBuilder, llvm::Value *framePtr, llvm::Value *value, u1 type) const;

        void createCallClinit(llvm::IRBuilder<> &irBuilder, llvm::Value *framePtr, llvm::Value *klass) const;
        llvm::Value *createCallInvokeMethodFixed(llvm::IRBuilder<> &irBuilder, llvm::Value *framePtr, llvm::Value *method, u2 paramSlotSize, u4 pc, cview methodName) const;
        llvm::Value *createCallNew(llvm::IRBuilder<> &irBuilder, llvm::Value *framePtr, uint8_t type, llvm::Value *length, llvm::Value *klass) const;
        void createCallThrowException(llvm::IRBuilder<> &irBuilder, llvm::Value *framePtr, llvm::Value *exception, u4 pc, u1 fixedException) const;
        llvm::Value * createCallInstanceOf(llvm::IRBuilder<> &irBuilder, llvm::Value *framePtr, u1 type, llvm::Value *oop, llvm::Value *checkClass) const;
        void createCallMonitor(llvm::IRBuilder<> &irBuilder, llvm::Value *oop, u1 type) const;

    };

}

#endif
