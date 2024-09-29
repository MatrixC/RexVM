#ifndef LLVM_COMPILER_HPP
#define LLVM_COMPILER_HPP
#include <stack>
#include <vector>
#include <map>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>
#include "../config.hpp"
#include "../opcode.hpp"
#include "llvm_jit_register_help_function.hpp"
#include "../utils/byte_reader.hpp"

namespace RexVM {
    struct ConstantInfo;
    struct InstanceClass;
    struct Method;
    struct LLVMHelpFunction;

    struct MethodCompiler {
        explicit MethodCompiler(
            Method &method,
            llvm::Module &module,
            cview compiledMethodName
        );

        void initBasicBlock();
        void initHelpFunction();
        void changeBB(llvm::BasicBlock *nextBasicBlock);

        Method &method;
        InstanceClass &klass;
        std::vector<std::unique_ptr<ConstantInfo> > &constantPool;

        llvm::Module &module;
        llvm::LLVMContext &ctx;
        llvm::IRBuilder<> irBuilder;
        std::unique_ptr<LLVMHelpFunction> helpFunction;

        llvm::PointerType *voidPtrType{};
        llvm::Function *function{};
        llvm::BasicBlock *returnBB{};
        llvm::BasicBlock *currentBB{};
        std::map<u4, llvm::BasicBlock *> opCodeBlocks;
        std::stack<llvm::Value *> valueStack;

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

        llvm::Type *slotTypeMap(SlotTypeEnum slotType);

        [[nodiscard]] llvm::Argument *getFramePtr() const;

        [[nodiscard]] llvm::Argument *getLocalVariableTablePtr() const;

        [[nodiscard]] llvm::Argument *getLocalVariableTableTypePtr() const;

        llvm::Value *getLocalVariableTableValue(u4 index, SlotTypeEnum slotType);

        void setLocalVariableTableValue(u4 index, llvm::Value *value, SlotTypeEnum slotType);

        void pushValue(llvm::Value *value);

        llvm::Value *popValue();

        llvm::Value *topValue();

        void pushI8Padding();

        void pushF8Padding();

        void pushI4Const(i4 value);

        void pushI8Const(i8 value);

        void pushF4Const(f4 value);

        void pushF8Const(f8 value);

        void pushNullConst();

        void throwNpeIfNull(llvm::Value *val);

        void ldc_helper(u2 index);

        void load(u4 index, SlotTypeEnum slotType);

        void store(u4 index, SlotTypeEnum slotType);

        void arrayLoad(llvm::Value *arrayRef, llvm::Value *index, uint8_t type);

        void arrayStore(llvm::Value *arrayRef, llvm::Value *index, llvm::Value *value, uint8_t type);

        void lCmp(llvm::Value *val1, llvm::Value *val2);

        void fCmp(llvm::Value *val1, llvm::Value *val2, llvm::Value *nanRet);

        void ifOp(u4 jumpTo, llvm::Value *val1, llvm::Value *val2, OpCodeEnum op);

        void processInstruction(OpCodeEnum opCode, u4 pc, ByteReader &byteReader);

        void compile();
    };
}

#endif
