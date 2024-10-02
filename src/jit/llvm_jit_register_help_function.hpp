#ifndef LLVM_JIT_REGISTER_HELP_FUNCTION_HPP
#define LLVM_JIT_REGISTER_HELP_FUNCTION_HPP
#include "../config.hpp"
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>


namespace RexVM {

    struct LLVMHelpFunction {
        explicit LLVMHelpFunction(llvm::Module &module);

        llvm::FunctionCallee getStringConstantHelper{};
        llvm::FunctionCallee getClassMirrorConstantHelper{};
        llvm::FunctionCallee throwNpeHelper{};

        llvm::FunctionCallee arrayLength{};
        llvm::FunctionCallee arrayLoadI4{};
        llvm::FunctionCallee arrayLoadI8{};
        llvm::FunctionCallee arrayLoadF4{};
        llvm::FunctionCallee arrayLoadF8{};
        llvm::FunctionCallee arrayLoadObj{};
        llvm::FunctionCallee arrayStoreI4{};
        llvm::FunctionCallee arrayStoreI8{};
        llvm::FunctionCallee arrayStoreF4{};
        llvm::FunctionCallee arrayStoreF8{};
        llvm::FunctionCallee arrayStoreObj{};

        llvm::FunctionCallee returnI4{};
        llvm::FunctionCallee returnI8{};
        llvm::FunctionCallee returnF4{};
        llvm::FunctionCallee returnF8{};
        llvm::FunctionCallee returnObj{};
        llvm::FunctionCallee returnVoid{};

        llvm::FunctionCallee clinit{};
        llvm::FunctionCallee getField{};
        llvm::FunctionCallee invokeMethod{};
        llvm::FunctionCallee invokeMethodStatic{};
        llvm::FunctionCallee newObject{};
        llvm::FunctionCallee throwException{};
        llvm::FunctionCallee instanceOf{};
        llvm::FunctionCallee monitor{};


        void createCallThrowNPE(llvm::IRBuilder<> &irBuilder, llvm::Value *framePtr, u4 pc) const;
        llvm::Value *createCallGetStringConstant(llvm::IRBuilder<> &irBuilder, llvm::Value *framePtr, u2 index) const;
        llvm::Value *createCallGetClassMirrorConstant(llvm::IRBuilder<> &irBuilder, llvm::Value *framePtr, u2 index) const;

        llvm::Value *createCallArrayLength(llvm::IRBuilder<> &irBuilder, llvm::Value *arrayRef) const;
        llvm::Value *createCallArrayLoadI4(llvm::IRBuilder<> &irBuilder, llvm::Value *arrayRef, llvm::Value *index, uint8_t type) const;
        llvm::Value *createCallArrayLoadI8(llvm::IRBuilder<> &irBuilder, llvm::Value *arrayRef, llvm::Value *index) const;
        llvm::Value *createCallArrayLoadF4(llvm::IRBuilder<> &irBuilder, llvm::Value *arrayRef, llvm::Value *index) const;
        llvm::Value *createCallArrayLoadF8(llvm::IRBuilder<> &irBuilder, llvm::Value *arrayRef, llvm::Value *index) const;
        llvm::Value *createCallArrayLoadObj(llvm::IRBuilder<> &irBuilder, llvm::Value *arrayRef, llvm::Value *index) const;

        void createCallArrayStoreI4(llvm::IRBuilder<> &irBuilder, llvm::Value *arrayRef, llvm::Value *index, llvm::Value *value, uint8_t type) const;
        void createCallArrayStoreI8(llvm::IRBuilder<> &irBuilder, llvm::Value *arrayRef, llvm::Value *index, llvm::Value *value) const;
        void createCallArrayStoreF4(llvm::IRBuilder<> &irBuilder, llvm::Value *arrayRef, llvm::Value *index, llvm::Value *value) const;
        void createCallArrayStoreF8(llvm::IRBuilder<> &irBuilder, llvm::Value *arrayRef, llvm::Value *index, llvm::Value *value) const;
        void createCallArrayStoreObj(llvm::IRBuilder<> &irBuilder, llvm::Value *arrayRef, llvm::Value *index, llvm::Value *value) const;

        void createCallReturnI4(llvm::IRBuilder<> &irBuilder, llvm::Value *framePtr, llvm::Value *value) const;
        void createCallReturnI8(llvm::IRBuilder<> &irBuilder, llvm::Value *framePtr, llvm::Value *value) const;
        void createCallReturnF4(llvm::IRBuilder<> &irBuilder, llvm::Value *framePtr, llvm::Value *value) const;
        void createCallReturnF8(llvm::IRBuilder<> &irBuilder, llvm::Value *framePtr, llvm::Value *value) const;
        void createCallReturnObj(llvm::IRBuilder<> &irBuilder, llvm::Value *framePtr, llvm::Value *value) const;
        void createCallReturnVoid(llvm::IRBuilder<> &irBuilder, llvm::Value *framePtr) const;

        void createCallClinit(llvm::IRBuilder<> &irBuilder, llvm::Value *framePtr, llvm::Value *klass) const;
        llvm::Value *createCallGetFieldPtr(llvm::IRBuilder<> &irBuilder, llvm::Value *klass, llvm::Value *index) const;
        void createCallInvokeMethod(llvm::IRBuilder<> &irBuilder, llvm::Value *framePtr, uint16_t index) const;
        void createCallInvokeMethodStatic(llvm::IRBuilder<> &irBuilder, llvm::Value *framePtr, llvm::Value *method, uint16_t paramSlotSize) const;
        llvm::Value *createCallNew(llvm::IRBuilder<> &irBuilder, llvm::Value *framePtr, uint8_t type, llvm::Value *length, llvm::Value *klass) const;
        void createCallThrowException(llvm::IRBuilder<> &irBuilder, llvm::Value *framePtr, llvm::Value *exception) const;
        llvm::Value * createCallInstanceOf(llvm::IRBuilder<> &irBuilder, llvm::Value *framePtr, u1 type, llvm::Value *oop, llvm::Value *checkClass) const;
        void createCallMonitor(llvm::IRBuilder<> &irBuilder, llvm::Value *oop, u1 type) const;

    };

}

#endif
