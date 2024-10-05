#ifndef LLVM_COMPILER_HPP
#define LLVM_COMPILER_HPP
#include <vector>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>
#include "../config.hpp"
#include "../opcode.hpp"
#include "../cfg.hpp"
#include "llvm_jit_register_help_function.hpp"
#include "llvm_block_context.hpp"

namespace RexVM {
    struct VM;
    struct ConstantInfo;
    struct InstanceClass;
    struct Method;
    struct Field;
    struct LLVMHelpFunction;
    struct BlockContext;

    struct MethodCompiler {
        explicit MethodCompiler(
            VM &vm,
            Method &method,
            llvm::Module &module,
            cview compiledMethodName
        );

        void initCFGBlocks();
        void changeBB(llvm::BasicBlock *nextBasicBlock);

        VM &vm;
        Method &method;
        InstanceClass &klass;
        std::vector<std::unique_ptr<ConstantInfo> > &constantPool;

        MethodCFG cfg;
        std::vector<std::unique_ptr<BlockContext>> cfgBlocks;

        llvm::Module &module;
        llvm::LLVMContext &ctx;
        llvm::IRBuilder<> irBuilder;
        std::unique_ptr<LLVMHelpFunction> helpFunction;

        llvm::PointerType *voidPtrType{};
        llvm::Function *function{};
        llvm::BasicBlock *currentBB{};
        llvm::BasicBlock *exitBB{};

        [[nodiscard]] BlockContext *getBlockContext(u4 leaderPC) const;

        llvm::Type *slotTypeMap(SlotTypeEnum slotType);

        llvm::Value *getZeroValue(SlotTypeEnum slotType);

        [[nodiscard]] llvm::Argument *getFramePtr() const;

        [[nodiscard]] llvm::Argument *getLocalVariableTablePtr() const;

        [[nodiscard]] llvm::Argument *getLocalVariableTableTypePtr() const;

        llvm::Value *getLocalVariableTableValue(u4 index, SlotTypeEnum slotType);

        void setLocalVariableTableValue(u4 index, llvm::Value *value, SlotTypeEnum slotType);

        llvm::Value * getOopDataPtr(llvm::Value *oop, llvm::Value *index, bool isArray, BasicType type);

        void returnValue(llvm::Value *val, SlotTypeEnum type);

        void throwNpeIfZero(const BlockContext &blockContext, llvm::Value *val, SlotTypeEnum slotType);

        void throwNpeIfNull(const BlockContext &blockContext, llvm::Value *val);

        void throwException(const BlockContext &blockContext, llvm::Value *ex);

        void ldc(BlockContext &blockContext, u2 index);

        void load(BlockContext &blockContext, u4 index, SlotTypeEnum slotType);

        void store(BlockContext &blockContext, u4 index, SlotTypeEnum slotType);

        void arrayLength(BlockContext &blockContext, llvm::Value *arrayRef);

        void arrayLoad(BlockContext &blockContext, llvm::Value *arrayRef, llvm::Value *index, uint8_t type);

        void arrayStore(const BlockContext &blockContext, llvm::Value *arrayRef, llvm::Value *index, llvm::Value *value, uint8_t type);

        void lCmp(BlockContext &blockContext, llvm::Value *val1, llvm::Value *val2);

        void fCmp(BlockContext &blockContext, llvm::Value *val1, llvm::Value *val2, llvm::Value *nanRet);

        static u4 offsetToPC(const BlockContext &blockContext, i4 offset);

        void ifOp(const BlockContext &blockContext, i4 offset, llvm::Value *val1, llvm::Value *val2, OpCodeEnum op);

        void jumpToPC(u4 pc);

        void jumpTo(const BlockContext &blockContext, i4 offset);

        void createSwitch(const BlockContext &blockContext, llvm::Value *val, std::vector<i4> cases);

        void exitMethod();

        llvm::Value *getConstantPtr(void *ptr);

        [[nodiscard]] std::tuple<Field *, void *> getFieldInfo(u2 index, bool isStatic) const;

        void getStatic(BlockContext &blockContext, u2 index);

        void putStatic(BlockContext &blockContext, u2 index);

        void getField(BlockContext &blockContext, u2 index);

        void putField(BlockContext &blockContext, u2 index);

        size_t pushParams(BlockContext &blockContext, const std::vector<cstring> &paramType, bool includeThis);

        void invokeStaticMethod(BlockContext &blockContext, u2 index, bool isStatic);

        void invokeVirtualMethod(BlockContext &blockContext, u2 index);

        void invokeDynamic(BlockContext &blockContext, u2 index);

        void newOpCode(BlockContext &blockContext, uint8_t type, llvm::Value *length, llvm::Value *klass);

        void newObject(BlockContext &blockContext, u2 index);

        void newArray(BlockContext &blockContext, u1 type, llvm::Value *length);

        void newObjectArray(BlockContext &blockContext, u2 index, llvm::Value *length);

        void newMultiArray(BlockContext &blockContext, u2 index, u1 dimension, llvm::Value *arrayDim);

        void checkCast(u2 index, llvm::Value *ref);

        void instanceOf(BlockContext &blockContext, u2 index, llvm::Value *ref);

        void monitor(const BlockContext &blockContext, u1 type, llvm::Value *oop);

        void processInvokeReturn(BlockContext &blockContext, cview returnType);

        void compile();
    };
}

#endif
