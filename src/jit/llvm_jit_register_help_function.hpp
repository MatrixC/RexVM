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

        void createCallThrowNPE(llvm::IRBuilder<> &irBuilder, llvm::Value *framePtr) const;
        llvm::Value *createCallGetStringConstant(llvm::IRBuilder<> &irBuilder, llvm::Value *framePtr, u2 index) const;
        llvm::Value *createCallGetClassMirrorConstant(llvm::IRBuilder<> &irBuilder, llvm::Value *framePtr, u2 index) const;

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

    };

}

#endif
