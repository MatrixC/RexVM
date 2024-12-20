#ifndef LLVM_COMPILER_HPP
#define LLVM_COMPILER_HPP
#include <vector>
#include <unordered_set>
#include <llvm/IR/IRBuilder.h>
#include "../basic.hpp"
#include "../opcode.hpp"
#include "../cfg.hpp"

namespace RexVM {
    struct VM;
    struct ConstantInfo;
    struct InstanceClass;
    struct Method;
    struct Field;
    struct LLVMHelpFunction;
    struct BlockContext;
    struct Class;

    struct MethodCompiler {
        explicit MethodCompiler(
            VM &vm,
            Method &method,
            llvm::Module &module,
            cview compiledMethodName
        );
        ~MethodCompiler();

        void initLocalPtr();
        void initCFGBlocks();
        void startBB(llvm::BasicBlock *nextBasicBlock);
        void changeBB(BlockContext &blockContext, llvm::BasicBlock *nextBasicBlock);

        VM &vm;
        Method &method;
        InstanceClass &klass;
        size_t localCount;
        std::vector<std::unique_ptr<ConstantInfo>> &constantPool;
        bool useLVTOptimize; //使用block的本地变量表(不写frame的栈内存 把llvm::Value保存在context的数组中)
        bool checkStackError; //是否做字节码执行后的栈数量检查 如果开启则函数编译结束后如果栈的元素数量不为0会报错
        bool useException;
        std::unordered_set<Class *> initClasses;

        MethodCFG cfg;
        std::vector<std::unique_ptr<BlockContext>> cfgBlocks;

        llvm::Module &module;
        llvm::LLVMContext &ctx;
        llvm::IRBuilder<> irBuilder;
        std::unique_ptr<LLVMHelpFunction> helpFunction;

        llvm::PointerType *voidPtrType{};
        llvm::Function *function{};
        llvm::BasicBlock *currentBB{};
        llvm::BasicBlock *entryBlock{};
        llvm::BasicBlock *exitBB{};
        llvm::Value *returnValuePtr{};
        std::vector<llvm::Value *> localPtr;
        std::vector<llvm::Value *> localTypePtr;
        std::vector<llvm::Value *> invokeMethodParamPtr;
        std::vector<llvm::Value *> invokeMethodParamTypePtr;

        [[nodiscard]] BlockContext *getBlockContext(u4 leaderPC) const;

        llvm::Type *slotTypeMap(SlotTypeEnum slotType);

        SlotTypeEnum llvmTypeMap(const llvm::Type *type);

        llvm::Value *getZeroValue(SlotTypeEnum slotType);

        llvm::Value *getZeroValue(const llvm::Type *type);;

        [[nodiscard]] llvm::Argument *getFramePtr() const;

        [[nodiscard]] llvm::Argument *getLocalVariableTablePtr() const;

        [[nodiscard]] llvm::Argument *getLocalVariableTableTypePtr() const;

        [[nodiscard]] llvm::Argument *getThrowValuePtr() const;

        llvm::Value *getLocalVariableTableValueMemory(u4 index, SlotTypeEnum slotType);

        void setLocalVariableTableValueMemory(u4 index, llvm::Value *value, SlotTypeEnum slotType);

        void writeModifyLocalVariableTable(const BlockContext &blockContext);

        llvm::Value *getLocalVariableTableValue(BlockContext &blockContext, u4 index, SlotTypeEnum slotType);

        void setLocalVariableTableValue(BlockContext &blockContext, u4 index, llvm::Value *value, SlotTypeEnum slotType);

        std::tuple<llvm::Type *, llvm::Value *> getOopDataPtr(llvm::Value *oop, llvm::Value *index, bool isArray, BasicType type);

        void returnValue(llvm::Value *val, SlotTypeEnum type);

        void throwIfZero(BlockContext &blockContext, llvm::Value *val, SlotTypeEnum slotType);

        void throwNpeIfNull(BlockContext &blockContext, llvm::Value *val);

        void throwException(BlockContext &blockContext, llvm::Value *ex);

        void ldc(BlockContext &blockContext, u2 index);

        void load(BlockContext &blockContext, u4 index, SlotTypeEnum slotType);

        void store(BlockContext &blockContext, u4 index, SlotTypeEnum slotType);

        void arrayLength(BlockContext &blockContext, llvm::Value *arrayRef);

        void arrayLoad(BlockContext &blockContext, llvm::Value *arrayRef, llvm::Value *index, uint8_t type);

        void arrayStore(BlockContext &blockContext, llvm::Value *arrayRef, llvm::Value *index, llvm::Value *value, uint8_t type);

        void lCmp(BlockContext &blockContext, llvm::Value *val1, llvm::Value *val2);

        void fCmp(BlockContext &blockContext, llvm::Value *val1, llvm::Value *val2, llvm::Value *nanRet);

        static u4 offsetToPC(const BlockContext &blockContext, i4 offset);

        void ifOp(const BlockContext &blockContext, i4 offset, llvm::Value *val1, llvm::Value *val2, OpCodeEnum op);

        void jumpToPC(u4 pc);

        void jumpTo(const BlockContext &blockContext, i4 offset);

        void createSwitch(const BlockContext &blockContext, llvm::Value *val, const std::vector<i4> &cases);

        void exitMethod();

        llvm::Value *getConstantPtr(void *ptr);

        [[nodiscard]] std::tuple<Field *, Slot *> getFieldInfo(u2 index, bool isStatic) const;

        void getOrPutStatic(BlockContext &blockContext, u2 index, u1 opType);

        void getField(BlockContext &blockContext, u2 index);

        void putField(BlockContext &blockContext, u2 index);

        size_t pushParams(BlockContext &blockContext, const std::vector<cstring> &paramType, bool includeThis);

        void invokeCommon(BlockContext &blockContext, cview methodName, cview returnType, llvm::Value *methodRef, size_t paramSlotSize);

        void invokeStaticMethod(BlockContext &blockContext, u2 index, bool isStatic);

        void invokeVirtualMethod(BlockContext &blockContext, u2 index);

        void invokeDynamic(BlockContext &blockContext, u2 index);

        void processInvokeReturn(BlockContext &blockContext, cview returnType);

        void newOpCode(BlockContext &blockContext, uint8_t type, llvm::Value *length, llvm::Value *klass);

        void newObject(BlockContext &blockContext, u2 index);

        void newArray(BlockContext &blockContext, u1 type, llvm::Value *length);

        void newObjectArray(BlockContext &blockContext, u2 index, llvm::Value *length);

        void newMultiArray(BlockContext &blockContext, u2 index, u1 dimension, llvm::Value *arrayDim);

        void checkCast(BlockContext &blockContext, u2 index, llvm::Value *ref);

        void instanceOf(BlockContext &blockContext, u2 index, llvm::Value *ref);

        void monitor(BlockContext &blockContext, u1 type, llvm::Value *oop);

        void iinc(BlockContext &blockContext, i4 index, i4 value);

        void initClass(BlockContext &blockContext,  InstanceClass *klass);

        void processCommonException(BlockContext &blockContext, llvm::Value *exception);

        void initCommonBlock();

        void addParamSlot(const BlockContext &blockContext, u4 paramCount);

        [[nodiscard]] llvm::Value *getInvokeReturnPtr(const BlockContext &blockContext) const;

        llvm::Value *loadThrowValue();

        void safePoint();

        bool compile();

        void verify() const;
    };
}

#endif
