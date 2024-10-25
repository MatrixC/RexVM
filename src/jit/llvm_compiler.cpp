#include "llvm_compiler.hpp"
#include <llvm/ExecutionEngine/JITLink/JITLink.h>
#include <llvm/IR/Verifier.h>
#include "llvm_block_context.hpp"
#include "jit_help_function.hpp"
#include "llvm_register_help_function.hpp"
#include "../vm.hpp"
#include "../frame.hpp"
#include "../class.hpp"
#include "../class_member.hpp"
#include "../oop.hpp"
#include "../class_loader.hpp"
#include "../constant_info.hpp"
#include "../method_handle.hpp"
#include "../utils/descriptor_parser.hpp"


namespace RexVM {
    using namespace llvm;

    // constexpr u2 INSTANCE_OOP_DATA_FIELD_OFFSET = offsetof(InstanceOop, data);
    // constexpr u2 ARRAY_OOP_DATA_FIELD_OFFSET = offsetof(ObjArrayOop, data);

    //release编译不支持Oop的offsetof
    constexpr u2 INSTANCE_OOP_DATA_FIELD_OFFSET = sizeof(Oop);
    constexpr u2 ARRAY_OOP_DATA_FIELD_OFFSET = sizeof(Oop);

    MethodCompiler::MethodCompiler(
        VM &vm,
        Method &method,
        Module &module,
        const cview compiledMethodName
    ) : vm(vm),
        method(method),
        klass(method.klass),
        localCount(method.maxLocals),
        constantPool(klass.constantPool),
        useLVTOptimize(vm.params.jitLVTOptimize),
        checkStackError(vm.params.jitCheckStack),
        useException(vm.params.jitSupportException),
        cfg(method),
        module(module),
        ctx(module.getContext()),
        irBuilder(ctx),
        helpFunction(std::make_unique<LLVMHelpFunction>(module)),
        voidPtrType(PointerType::getUnqual(irBuilder.getVoidTy())),
        localPtr(localCount, nullptr),
        localTypePtr(localCount, nullptr) {

        const auto functionType =
                FunctionType::get(
                    irBuilder.getVoidTy(),
                    {voidPtrType, voidPtrType, voidPtrType, voidPtrType},
                    false
                );

        function = Function::Create(
            functionType,
            GlobalVariable::LinkageTypes::ExternalLinkage,
            compiledMethodName,
            module
        );

        entryBlock = BasicBlock::Create(ctx, "entry", function);
        exitBB = BasicBlock::Create(ctx, "exit_method");

        initLocalPtr();
        initCFGBlocks();
    }

    MethodCompiler::~MethodCompiler() = default;


    void MethodCompiler::initLocalPtr() {
        irBuilder.SetInsertPoint(entryBlock);
        //初始化LVT的指针
        for (size_t i = 0; i < localCount; ++i) {
            const auto indexValue = irBuilder.getInt32(i);

            const auto lvtPtr =
                    irBuilder.CreateGEP(
                        irBuilder.getInt64Ty(),
                        getLocalVariableTablePtr(),
                        indexValue,
                        cformat("local_ptr_{}", i)
                    );

            const auto lvtTypePtr =
                    irBuilder.CreateGEP(
                        irBuilder.getInt8Ty(),
                        getLocalVariableTableTypePtr(),
                        indexValue,
                        cformat("local_type_ptr_{}", i)
                    );

            localPtr[i] = lvtPtr;
            localTypePtr[i] = lvtTypePtr;
        }

        //初始化调用函数后返回结果的指针
        returnValuePtr =
                irBuilder.CreateGEP(
                    irBuilder.getInt64Ty(),
                    getLocalVariableTablePtr(),
                    irBuilder.getInt32(localCount),
                    "invoke_return_ptr"
                );
    }


    void MethodCompiler::initCFGBlocks() {
        cfgBlocks.reserve(cfg.blocks.size());
        for (const auto &cfgBlock : cfg.blocks) {
            const auto lineNumber = method.getLineNumber(cfgBlock->startPC);
            const auto blockName = cformat("{}_{}", cfgBlock->startPC, lineNumber);
            cfgBlocks.emplace_back(std::make_unique<BlockContext>(*this, cfgBlock.get()));
        }

        for (size_t i = 0; i < cfg.blocks.size(); ++i) {
            const auto currentBlock = cfg.blocks[i].get();
            for (const auto parents = currentBlock->parentBlockIndex; const auto parentIdx: parents) {
                cfgBlocks[i]->parentBlocks.emplace_back(cfgBlocks[parentIdx].get());
            }

            for (const auto jumpTos = currentBlock->jumpToBlockIndex; const auto jumpToIdx: jumpTos) {
                cfgBlocks[i]->jumpToBlocks.emplace_back(cfgBlocks[jumpToIdx].get());
            }
        }

    }

    void MethodCompiler::startBB(BasicBlock *nextBasicBlock) {
        currentBB = nextBasicBlock;
        nextBasicBlock->insertInto(function);
        irBuilder.SetInsertPoint(nextBasicBlock);
    }


    void MethodCompiler::changeBB(BlockContext &blockContext, BasicBlock *nextBasicBlock) {
        if (currentBB != nullptr) {
            if (const auto &lastInstr = currentBB->back();
                currentBB->empty() || !lastInstr.isTerminator()) {
                irBuilder.CreateBr(nextBasicBlock);
            }
        }
        blockContext.lastBasicBlock = nextBasicBlock;
        startBB(nextBasicBlock);
    }


    Argument *MethodCompiler::getFramePtr() const {
        return function->arg_begin();
    }

    Argument *MethodCompiler::getLocalVariableTablePtr() const {
        return function->arg_begin() + 1;
    }

    Argument *MethodCompiler::getLocalVariableTableTypePtr() const {
        return function->arg_begin() + 2;
    }

    Argument *MethodCompiler::getThrowValuePtr() const {
        return function->arg_begin() + 3;
    }

    BlockContext *MethodCompiler::getBlockContext(const u4 leaderPC) const {
        for (const auto &block : cfgBlocks) {
            if (block->methodBlock->startPC == leaderPC) {
                return block.get();
            }
        }
        panic("error leaderPC");
        return nullptr;
    }

    Type *MethodCompiler::slotTypeMap(const SlotTypeEnum slotType) {
        switch (slotType) {
            case SlotTypeEnum::I4:
                return irBuilder.getInt32Ty();
            case SlotTypeEnum::I8:
                return irBuilder.getInt64Ty();
            case SlotTypeEnum::F4:
                return irBuilder.getFloatTy();
            case SlotTypeEnum::F8:
                return irBuilder.getDoubleTy();
            case SlotTypeEnum::REF:
                return voidPtrType;
            default:
                panic("error slot type");
                return nullptr;
        }
    }

    SlotTypeEnum MethodCompiler::llvmTypeMap(const Type *type) {
        if (type == voidPtrType) {
            return (SlotTypeEnum::REF);
        }
        if (type == irBuilder.getInt32Ty()) {
            return (SlotTypeEnum::I4);
        }
        if (type == irBuilder.getInt64Ty()) {
            return (SlotTypeEnum::I8);
        }
        if (type == irBuilder.getFloatTy()) {
            return (SlotTypeEnum::F4);
        }
        if (type == irBuilder.getDoubleTy()) {
            return (SlotTypeEnum::F8);
        }
        panic("error type");
        return SlotTypeEnum::NONE;
    }


    llvm::Value *MethodCompiler::getZeroValue(const SlotTypeEnum slotType) {
        switch (slotType) {
            case SlotTypeEnum::I4:
                return irBuilder.getInt32(0);
            case SlotTypeEnum::I8:
                return irBuilder.getInt64(0);
            case SlotTypeEnum::F4:
                return ConstantFP::getZero(irBuilder.getFloatTy());
            case SlotTypeEnum::F8:
                return  ConstantFP::getZero(irBuilder.getDoubleTy());
            case SlotTypeEnum::REF:
                return ConstantPointerNull::get(voidPtrType);
            case SlotTypeEnum::NONE:
                panic("error slot type");
                break;
        }
        return nullptr;
    }

    llvm::Value *MethodCompiler::getZeroValue(const Type *type) {
        return getZeroValue(llvmTypeMap(type));
    }

    llvm::Value *MethodCompiler::getLocalVariableTableValueMemory(const u4 index, const SlotTypeEnum slotType) {
        //lvt指向Slot[] 先通过GEP获得index对应的Slot指针
        //在解释器中 因为op stack中的也全都是slot 所以不用做转换
        //但在jit中 Value都是具体的类型 所以需要做转换
        //使用createLoad可以自动转换成对应的类型 代码中会生成align信息
        const auto type = slotTypeMap(slotType);
        const auto ptr = localPtr[index];
        return irBuilder.CreateLoad(type, ptr, cformat("local_{}", index));
    }

    void MethodCompiler::setLocalVariableTableValueMemory(const u4 index, llvm::Value *value, SlotTypeEnum slotType) {
        // 写lvt和lvtType
        const auto lvtPtr = localPtr[index];
        const auto lvtTypePtr = localTypePtr[index];
        irBuilder.CreateStore(value, lvtPtr);
        irBuilder.CreateStore(irBuilder.getInt8(static_cast<uint8_t>(slotType)), lvtTypePtr);
    }

    void MethodCompiler::writeModifyLocalVariableTable(const BlockContext &blockContext) {
        if (!useLVTOptimize) {
            return;
        }

        for (size_t i = 0; i < localCount; ++i) {
            if (blockContext.wroteLocalVariableTable[i] != 0) {
                const auto curVal = blockContext.localVariableTable[i];
                const auto curValType = llvmTypeMap(curVal->getType());
                setLocalVariableTableValueMemory(i, curVal, curValType);
            }
        }
    }

    llvm::Value *MethodCompiler::getLocalVariableTableValue(BlockContext &blockContext, const u4 index, const SlotTypeEnum slotType) {
        llvm::Value *loadVal{nullptr};
        if (useLVTOptimize) {
            loadVal = blockContext.localVariableTable[index];
            if (loadVal == nullptr) {
                loadVal = getLocalVariableTableValueMemory(index, slotType);
                blockContext.localVariableTable[index] = loadVal;
            }
        } else {
            loadVal = getLocalVariableTableValueMemory(index, slotType);
        }
        return loadVal;
    }

    void MethodCompiler::setLocalVariableTableValue(BlockContext &blockContext, const u4 index, llvm::Value *value, const SlotTypeEnum slotType) {
        if (useLVTOptimize) {
            blockContext.localVariableTable[index] = value;
            blockContext.wroteLocalVariableTable[index] = 1;
        } else {
            setLocalVariableTableValueMemory(index, value, slotType);
        }
    }

    std::tuple<Type *, llvm::Value *> MethodCompiler::getOopDataPtr(llvm::Value *oop, llvm::Value *index, const bool isArray, const BasicType type) {
        const auto offset =
            irBuilder.getInt32(isArray ? ARRAY_OOP_DATA_FIELD_OFFSET : INSTANCE_OOP_DATA_FIELD_OFFSET);

        //先通过oop和偏移量 获取到data字段的地址
        const auto oopDataFieldPtr =
                irBuilder.CreateGEP(irBuilder.getInt8Ty(), oop, offset);

        const auto dataFieldValue = irBuilder.CreateLoad(voidPtrType, oopDataFieldPtr);
        //InstanceOop element is Slot
        const u4 elementByteSize = isArray ? getElementSizeByBasicType(type) : SLOT_BYTE_SIZE;
        const u4 elementBitSize = elementByteSize * 8;
        const auto dataPtrType = irBuilder.getIntNTy(elementBitSize);

        //再通过slotId或者数据的index 获取具体数据的地址
        const auto dataFieldPtr =
                irBuilder.CreateGEP(
                    dataPtrType,
                    dataFieldValue,
                    index
                );

        return std::make_tuple(dataPtrType, dataFieldPtr);
    }

    void MethodCompiler::returnValue(llvm::Value *val, SlotTypeEnum type) {
        llvm::Value *returnVal = nullptr;
        switch (type) {
            case SlotTypeEnum::NONE:
                returnVal = getZeroValue(SlotTypeEnum::I8);
                break;

            case SlotTypeEnum::I4:
                returnVal = irBuilder.CreateSExt(val, slotTypeMap(SlotTypeEnum::I8));
                break;

            case SlotTypeEnum::F4: {
                const auto int32Val = irBuilder.CreateBitCast(val, irBuilder.getInt32Ty());
                returnVal = irBuilder.CreateZExt(int32Val, slotTypeMap(SlotTypeEnum::I8));
                break;
            }

            case SlotTypeEnum::I8:
                returnVal = val;
                break;

            case SlotTypeEnum::F8:
                returnVal = irBuilder.CreateBitCast(val, irBuilder.getInt64Ty());
                break;
            case SlotTypeEnum::REF:
                returnVal = irBuilder.CreatePtrToInt(val, irBuilder.getInt64Ty());
                break;
        }
        helpFunction->createCallReturnCommon(irBuilder, getFramePtr(), returnVal, static_cast<u1>(type));
        exitMethod();
    }

    void MethodCompiler::throwIfZero(BlockContext &blockContext, llvm::Value *val, const SlotTypeEnum slotType) {
        // if (value == zero) {
        //   throwNpe(frame);
        //   return;
        // }
        const auto zeroValue = getZeroValue(slotType);
        const auto fixedException =
                slotType == SlotTypeEnum::REF
                    ? LLVM_COMPILER_FIXED_EXCEPTION_NPE
                    : LLVM_COMPILER_FIXED_EXCEPTION_DIV_BY_ZERO;

        const auto isNullBB = BasicBlock::Create(ctx);
        const auto elseBB = BasicBlock::Create(ctx);

        const auto cmpIsNull = irBuilder.CreateICmpEQ(val, zeroValue);
        irBuilder.CreateCondBr(cmpIsNull, isNullBB, elseBB);

        changeBB(blockContext, isNullBB);
        //不管是否要处理异常 都得先抛出异常
        //如果有处理块 则尝试处理 否则直接退出函数
        helpFunction->createCallThrowException(
            irBuilder,
            getFramePtr(),
            getZeroValue(SlotTypeEnum::REF),
            blockContext.pc,
            fixedException,
            getZeroValue(SlotTypeEnum::REF)
        );

        if (useException) {
            //处理异常
            //在已经知道要发生异常的场景下 先找异常处理程序 如果能找到则直接处理 就不用抛异常了
            //如果找不到 再抛异常
            const auto fixedExceptionClass =
                slotType == SlotTypeEnum::REF
                    ? method.klass.classLoader.getInstanceClass("java/lang/NullPointerException")
                    : method.klass.classLoader.getInstanceClass("java/lang/ArithmeticException");

            if (const auto catchBlocks = cfg.findCatchBlock(blockContext.pc, fixedExceptionClass); !catchBlocks.empty()) {
                const auto catchBlock = catchBlocks.back();
                const auto blkCtx = cfgBlocks[catchBlock->index].get();
                //异常因为要跳转 所以写入内存
                writeModifyLocalVariableTable(blockContext);
                irBuilder.CreateBr(blkCtx->basicBlock);
            } else {
                exitMethod();
            }
            //处理异常end
        } else {
            exitMethod();
        }

        changeBB(blockContext, elseBB);
    }


    void MethodCompiler::throwNpeIfNull(BlockContext &blockContext, llvm::Value *val) {
        throwIfZero(blockContext, val, SlotTypeEnum::REF);
    }

    void MethodCompiler::throwException(BlockContext &blockContext, llvm::Value *ex) {
        throwNpeIfNull(blockContext, ex);
        helpFunction->createCallThrowException(
            irBuilder,
            getFramePtr(),
            ex,
            blockContext.pc,
            LLVM_COMPILER_FIXED_EXCEPTION_NONE,
            getZeroValue(SlotTypeEnum::REF)
        );
        exitMethod();
    }

    void MethodCompiler::ldc(BlockContext &blockContext, const u2 index) {
        switch (const auto valPtr = constantPool[index].get();
            CAST_CONSTANT_TAG_ENUM(valPtr->tag)) {
            case ConstantTagEnum::CONSTANT_Integer:
                blockContext.pushI4Const((CAST_CONSTANT_INTEGER_INFO(valPtr))->value);
                break;

            case ConstantTagEnum::CONSTANT_Long:
                blockContext.pushI8Const((CAST_CONSTANT_LONG_INFO(valPtr))->value);
                break;

            case ConstantTagEnum::CONSTANT_Float:
                blockContext.pushF4Const((CAST_CONSTANT_FLOAT_INFO(valPtr))->value);
                break;

            case ConstantTagEnum::CONSTANT_Double:
                blockContext.pushF8Const((CAST_CONSTANT_DOUBLE_INFO(valPtr))->value);
                break;

            case ConstantTagEnum::CONSTANT_String:
            case ConstantTagEnum::CONSTANT_Class: {
                blockContext.pushValue(helpFunction->createCallGetInstanceConstant(irBuilder, getFramePtr(), index));
                break;
            }

            default:
                panic(cformat("ldc error tag{}", valPtr->tag));
        }
    }


    void MethodCompiler::load(BlockContext &blockContext, const u4 index, const SlotTypeEnum slotType) {
        blockContext.pushValue(getLocalVariableTableValue(blockContext, index, slotType), slotType);
    }

    void MethodCompiler::store(BlockContext &blockContext, const u4 index, const SlotTypeEnum slotType) {
        const auto value = blockContext.popValue(slotType);
        setLocalVariableTableValue(blockContext, index, value, slotType);
    }

    void MethodCompiler::arrayLength(BlockContext &blockContext, llvm::Value *arrayRef) {
        throwNpeIfNull(blockContext, arrayRef);
        const auto arrayLength =
                helpFunction->createCallMisc(
                    irBuilder,
                    getFramePtr(),
                    arrayRef,
                    getZeroValue(SlotTypeEnum::REF),
                    LLVM_COMPILER_MISC_ARRAY_LENGTH
                );
        blockContext.pushValue(arrayLength);
    }

    void MethodCompiler::arrayLoad(BlockContext &blockContext, llvm::Value *arrayRef, llvm::Value *index,
                                   const uint8_t type) {
        //arrayRef 是 arrayOop
        throwNpeIfNull(blockContext, arrayRef);
        llvm::Type *dataType{nullptr};
        llvm::Value *dataPtr{nullptr};
        switch (type) {
            case LLVM_COMPILER_INT_ARRAY_TYPE: {
                std::tie(dataType, dataPtr) = getOopDataPtr(arrayRef, index, true, BasicType::T_INT);
                blockContext.pushValue(irBuilder.CreateLoad(slotTypeMap(SlotTypeEnum::I4), dataPtr));
                break;
            }

            case LLVM_COMPILER_BYTE_ARRAY_TYPE: {
                std::tie(dataType, dataPtr) = getOopDataPtr(arrayRef, index, true, BasicType::T_BYTE);
                const auto oriValue = irBuilder.CreateLoad(dataType, dataPtr);
                const auto i4Value = irBuilder.CreateZExt(oriValue, slotTypeMap(SlotTypeEnum::I4));
                blockContext.pushValue(i4Value);
                break;
            }

            case LLVM_COMPILER_CHAR_ARRAY_TYPE: {
                std::tie(dataType, dataPtr) = getOopDataPtr(arrayRef, index, true, BasicType::T_CHAR);
                const auto oriValue = irBuilder.CreateLoad(dataType, dataPtr);
                const auto i4Value = irBuilder.CreateZExt(oriValue, slotTypeMap(SlotTypeEnum::I4));
                blockContext.pushValue(i4Value);
                break;
            }

            case LLVM_COMPILER_SHORT_ARRAY_TYPE: {
                std::tie(dataType, dataPtr) = getOopDataPtr(arrayRef, index, true, BasicType::T_SHORT);
                const auto oriValue = irBuilder.CreateLoad(dataType, dataPtr);
                const auto i4Value = irBuilder.CreateSExt(oriValue, slotTypeMap(SlotTypeEnum::I4));
                blockContext.pushValue(i4Value);
                break;
            }

            case LLVM_COMPILER_LONG_ARRAY_TYPE: {
                std::tie(dataType, dataPtr) = getOopDataPtr(arrayRef, index, true, BasicType::T_LONG);
                blockContext.pushWideValue(irBuilder.CreateLoad(slotTypeMap(SlotTypeEnum::I8), dataPtr));
                break;
            }

            case LLVM_COMPILER_FLOAT_ARRAY_TYPE: {
                std::tie(dataType, dataPtr) = getOopDataPtr(arrayRef, index, true, BasicType::T_FLOAT);
                blockContext.pushValue(irBuilder.CreateLoad(slotTypeMap(SlotTypeEnum::F4), dataPtr));
                break;
            }

            case LLVM_COMPILER_DOUBLE_ARRAY_TYPE: {
                std::tie(dataType, dataPtr) = getOopDataPtr(arrayRef, index, true, BasicType::T_DOUBLE);
                blockContext.pushWideValue(irBuilder.CreateLoad(slotTypeMap(SlotTypeEnum::F8), dataPtr));
                break;
            }
            case LLVM_COMPILER_OBJ_ARRAY_TYPE: {
                std::tie(dataType, dataPtr) = getOopDataPtr(arrayRef, index, true, BasicType::T_OBJECT);
                blockContext.pushValue(irBuilder.CreateLoad(slotTypeMap(SlotTypeEnum::REF), dataPtr));
                break;
            }

            default:
                panic("error type");
        }
    }

    void MethodCompiler::arrayStore(BlockContext &blockContext, llvm::Value *arrayRef, llvm::Value *index, llvm::Value *value, const uint8_t type) {
        throwNpeIfNull(blockContext, arrayRef);
        llvm::Type *dataType{nullptr};
        llvm::Value *dataPtr{nullptr};
         switch (type) {
            case LLVM_COMPILER_INT_ARRAY_TYPE:
                std::tie(dataType, dataPtr) = getOopDataPtr(arrayRef, index, true, BasicType::T_INT);
                break;

            case LLVM_COMPILER_BYTE_ARRAY_TYPE:
                std::tie(dataType, dataPtr) = getOopDataPtr(arrayRef, index, true, BasicType::T_BYTE);
                value = irBuilder.CreateTrunc(value, dataType);
                break;

            case LLVM_COMPILER_CHAR_ARRAY_TYPE:
                std::tie(dataType, dataPtr) = getOopDataPtr(arrayRef, index, true, BasicType::T_CHAR);
                value = irBuilder.CreateTrunc(value, dataType);
                break;

            case LLVM_COMPILER_SHORT_ARRAY_TYPE:
                std::tie(dataType, dataPtr) = getOopDataPtr(arrayRef, index, true, BasicType::T_SHORT);
                value = irBuilder.CreateTrunc(value, dataType);
                break;

            case LLVM_COMPILER_LONG_ARRAY_TYPE:
                std::tie(dataType, dataPtr) = getOopDataPtr(arrayRef, index, true, BasicType::T_LONG);
                break;

            case LLVM_COMPILER_FLOAT_ARRAY_TYPE:
                std::tie(dataType, dataPtr) = getOopDataPtr(arrayRef, index, true, BasicType::T_FLOAT);
                break;

            case LLVM_COMPILER_DOUBLE_ARRAY_TYPE:
                std::tie(dataType, dataPtr) = getOopDataPtr(arrayRef, index, true, BasicType::T_DOUBLE);
                break;

            case LLVM_COMPILER_OBJ_ARRAY_TYPE:
                std::tie(dataType, dataPtr) = getOopDataPtr(arrayRef, index, true, BasicType::T_OBJECT);
                break;

            default:
                panic("error type");
        }

        irBuilder.CreateStore(value, dataPtr);
    }

    void MethodCompiler::lCmp(BlockContext &blockContext, llvm::Value *val1, llvm::Value *val2) {
        const auto ifGt = BasicBlock::Create(ctx);
        const auto ifElse = BasicBlock::Create(ctx);
        const auto ifEq = BasicBlock::Create(ctx);
        const auto ifEqElse = BasicBlock::Create(ctx);
        const auto ifEnd = BasicBlock::Create(ctx);

        const auto cmpGt = irBuilder.CreateICmpSGT(val1, val2);
        irBuilder.CreateCondBr(cmpGt, ifGt, ifElse);

        changeBB(blockContext, ifGt);
        irBuilder.CreateBr(ifEnd);

        changeBB(blockContext, ifElse);
        const auto cmpEq = irBuilder.CreateICmpEQ(val1, val2);
        irBuilder.CreateCondBr(cmpEq, ifEq, ifEqElse);

        changeBB(blockContext, ifEq);
        irBuilder.CreateBr(ifEnd);

        changeBB(blockContext, ifEqElse);
        irBuilder.CreateBr(ifEnd);

        changeBB(blockContext, ifEnd);
        const auto phi = irBuilder.CreatePHI(irBuilder.getInt32Ty(), 3);
        phi->addIncoming(irBuilder.getInt32(1), ifGt);
        phi->addIncoming(irBuilder.getInt32(0), ifEq);
        phi->addIncoming(irBuilder.getInt32(-1), ifEqElse);

        blockContext.pushValue(phi);
    }

    void MethodCompiler::fCmp(BlockContext &blockContext, llvm::Value *val1, llvm::Value *val2, llvm::Value *nanRet) {
        const auto ifGt = BasicBlock::Create(ctx);
        const auto ifGtElse = BasicBlock::Create(ctx);
        const auto ifEq = BasicBlock::Create(ctx);
        const auto ifEqElse = BasicBlock::Create(ctx);
        const auto ifLt = BasicBlock::Create(ctx);
        const auto ifNan = BasicBlock::Create(ctx);
        const auto ifEnd = BasicBlock::Create(ctx);

        const auto cmpGt = irBuilder.CreateFCmpOGT(val1, val2);
        irBuilder.CreateCondBr(cmpGt, ifGt, ifGtElse);

        changeBB(blockContext, ifGt);
        irBuilder.CreateBr(ifEnd);

        changeBB(blockContext, ifGtElse);
        const auto cmpEq = irBuilder.CreateFCmpOEQ(val1, val2);
        irBuilder.CreateCondBr(cmpEq, ifEq, ifEqElse);

        changeBB(blockContext, ifEq);
        irBuilder.CreateBr(ifEnd);

        changeBB(blockContext, ifEqElse);
        const auto cmpLt = irBuilder.CreateFCmpOLT(val1, val2);
        irBuilder.CreateCondBr(cmpLt, ifLt, ifNan);

        changeBB(blockContext, ifLt);
        irBuilder.CreateBr(ifEnd);

        changeBB(blockContext, ifNan);
        irBuilder.CreateBr(ifEnd);

        changeBB(blockContext, ifEnd);
        const auto phi = irBuilder.CreatePHI(irBuilder.getInt32Ty(), 3);
        phi->addIncoming(irBuilder.getInt32(1), ifGt);
        phi->addIncoming(irBuilder.getInt32(0), ifEq);
        phi->addIncoming(irBuilder.getInt32(-1), ifLt);
        phi->addIncoming(nanRet, ifNan);

        blockContext.pushValue(phi);
    }

    u4 MethodCompiler::offsetToPC(const BlockContext &blockContext, const i4 offset) {
        return CAST_U4(CAST_I4(blockContext.pc) + offset);
    }

    void MethodCompiler::ifOp(const BlockContext &blockContext, const i4 offset, llvm::Value *val1, llvm::Value *val2, const OpCodeEnum op) {
        const auto jumpToBB = getBlockContext(offsetToPC(blockContext, offset))->basicBlock;
        // const auto nextOpCodeBB = cfgBlocks[blockContext.methodBlock->index + 1]->basicBlock;
        //下一个block的起始opCode偏移量是3 if本身占一个 readI2的offset占2个
        const auto nextOpCodeBB = getBlockContext(offsetToPC(blockContext, 3))->basicBlock;
        llvm::Value *cmp{nullptr};
        switch (op) {
            case OpCodeEnum::IFEQ:
                cmp = irBuilder.CreateICmpEQ(val1, val2);
                break;
            case OpCodeEnum::IFNE:
                cmp = irBuilder.CreateICmpNE(val1, val2);
                break;

            case OpCodeEnum::IFLT:
                cmp = irBuilder.CreateICmpSLT(val1, val2);
                break;

            case OpCodeEnum::IFGE:
                cmp = irBuilder.CreateICmpSGE(val1, val2);
                break;

            case OpCodeEnum::IFGT:
                cmp = irBuilder.CreateICmpSGT(val1, val2);
                break;

            case OpCodeEnum::IFLE:
                cmp = irBuilder.CreateICmpSLE(val1, val2);
                break;

            default:
                panic("error type");
                break;
        }
        //const auto elseBB = BasicBlock::Create(ctx);
        const auto elseBB = nextOpCodeBB;
        irBuilder.CreateCondBr(cmp, jumpToBB, elseBB);
        // changeBB(blockContext, elseBB);
    }

    void MethodCompiler::jumpToPC(const u4 pc) {
        const auto jumpToBB = getBlockContext(pc)->basicBlock;
        irBuilder.CreateBr(jumpToBB);
    }

    void MethodCompiler::jumpTo(const BlockContext &blockContext, const i4 offset) {
        const auto jumpTo = offsetToPC(blockContext, offset);
        jumpToPC(jumpTo);
    }

    void MethodCompiler::createSwitch(const BlockContext &blockContext, llvm::Value *val, const std::vector<i4> &cases) {
        const auto defaultOffset = cases[0];
        const auto defaultBlock = getBlockContext(offsetToPC(blockContext, defaultOffset))->basicBlock;

        const auto switchIr = irBuilder.CreateSwitch(val, defaultBlock);
        if (cases.size() > 1) {
            for (size_t i = 1; i < cases.size(); i+= 2) {
                const auto caseVal = irBuilder.getInt32(cases[i]);
                const auto caseOffset = cases[i + 1];
                if (caseOffset == defaultOffset) {
                    //omit
                    continue;
                }
                const auto caseBB = getBlockContext(offsetToPC(blockContext, caseOffset))->basicBlock;
                switchIr->addCase(caseVal, caseBB);
            }
        }
    }

    void MethodCompiler::exitMethod() {
        irBuilder.CreateBr(exitBB);
    }

    llvm::Value *MethodCompiler::getConstantPtr(void *ptr) {
        const auto ptrInt = irBuilder.getInt64(std::bit_cast<uint64_t>(ptr));
        return ConstantExpr::getIntToPtr(ptrInt, voidPtrType);
    }

    std::tuple<Field *, Slot *> MethodCompiler::getFieldInfo(const u2 index, const bool isStatic) const {
        const auto fieldRef = klass.getRefField(index, isStatic);
        const auto fieldClass = &fieldRef->klass;
        const auto dataPtr = isStatic ? &fieldClass->staticData[fieldRef->slotId] : nullptr;
        return std::make_tuple(fieldRef, dataPtr);
    }

    void MethodCompiler::getOrPutStatic(BlockContext &blockContext, const u2 index, const u1 opType) {
        const auto [field, dataPtr] = getFieldInfo(index, true);
        const auto type = field->getFieldSlotType();
        const auto klassPtr = &field->klass;
        const auto llvmDataPtr = getConstantPtr(dataPtr);
        initClass(blockContext, klassPtr);

        if (opType == 0) {
            const auto value = irBuilder.CreateLoad(slotTypeMap(type), llvmDataPtr, field->getName());
            blockContext.pushValue(value, type);
        } else {
            const auto value = blockContext.popValue(type);
            irBuilder.CreateStore(value, llvmDataPtr);
        }
    }

    void MethodCompiler::getField(BlockContext &blockContext, const u2 index) {
        const auto [field, ignored] = getFieldInfo(index, false);
        const auto type = field->getFieldSlotType();
        const auto slotId = field->slotId;
        const auto oop = blockContext.popValue();
        throwNpeIfNull(blockContext, oop);
        const auto [fieldDataType, fieldDataPtr] = getOopDataPtr(oop, irBuilder.getInt32(slotId), false, BasicType::T_OBJECT);
        const auto value = irBuilder.CreateLoad(slotTypeMap(type), fieldDataPtr, field->getName());
        blockContext.pushValue(value, type);
    }

    void MethodCompiler::putField(BlockContext &blockContext, const u2 index) {
        const auto [field, ignored] = getFieldInfo(index, false);
        const auto type = field->getFieldSlotType();
        const auto slotId = field->slotId;
        const auto value = blockContext.popValue(type);
        const auto oop = blockContext.popValue();
        throwNpeIfNull(blockContext, oop);
        const auto [fieldDataType, fieldDataPtr] = getOopDataPtr(oop, irBuilder.getInt32(slotId), false, BasicType::T_OBJECT);
        irBuilder.CreateStore(value, fieldDataPtr);
    }

    size_t MethodCompiler::pushParams(BlockContext &blockContext, const std::vector<cstring> &paramType, const bool includeThis) {
        //调用函数 首先需要将函数的参数push到操作数栈中 再由下一个函数pop出参数后执行
        //所以首先需要计算参数的数量 然后从valueStack中pop出来后 写入当前函数的操作数栈内存
        //includeThis参数代表计算时是否考虑this指针 主要区分是否是static调用
        std::vector<SlotTypeEnum> paramSlotType;
        if (includeThis) {
            paramSlotType.emplace_back(SlotTypeEnum::REF);
        }
        for (const auto &desc: paramType) {
            const auto slotType = getSlotTypeByPrimitiveClassName(desc);
            paramSlotType.emplace_back(slotType);
            if (isWideSlotType(slotType)) {
                paramSlotType.emplace_back(slotType);
            }
        }

        //由于JIT模式下的函数不需要使用操作数栈进行计算 所以操作数栈只用来做函数传参 传参时用首地址即可
        //根据Frame的初始化定义 操作数栈的首地址 operandStackPtr = lvtPtr + lvtSize
        //对于普通java函数 lvtSize = localCount 所以可以直接计算出操作数栈的地址
        //计算出操作数栈地址后 从后向前依次出valueStack 依次写入操作数栈(std::stack 不能按index读取)
        addParamSlot(blockContext, paramSlotType.size());
        for (i4 i = CAST_I4(paramSlotType.size()) - 1; i >= 0; --i) {
            //假设有4个参数 则 i = 3,2,1,0
            //具体的参数
            const auto paramValue = blockContext.popValue();
            if (includeThis && i == 0) {
                //对于需要传this的场景 要判断传入是否为null 为null报npe
                throwNpeIfNull(blockContext, paramValue);
            }

            const auto slotPtr = invokeMethodParamPtr[i];
            const auto slotTypePtr = invokeMethodParamTypePtr[i];

            if (paramValue != nullptr) {
                //nullptr 可能是padding
                irBuilder.CreateStore(paramValue, slotPtr);
            }

            const auto slotType = paramSlotType[i];
            irBuilder.CreateStore(irBuilder.getInt8(static_cast<uint8_t>(slotType)), slotTypePtr);
        }
        return paramSlotType.size();
    }

    void MethodCompiler::invokeCommon(
        BlockContext &blockContext,
        const cview methodName,
        const cview returnType,
        llvm::Value *methodRef,
        const size_t paramSlotSize
    ) {
        const auto invokeException =
                helpFunction->createCallInvokeMethodFixed(
                    irBuilder,
                    getFramePtr(),
                    methodRef,
                    paramSlotSize,
                    blockContext.pc,
                    methodName,
                    false
                );

        processCommonException(blockContext, invokeException);

        processInvokeReturn(blockContext, returnType);
    }

    void MethodCompiler::invokeStaticMethod(BlockContext &blockContext, const u2 index, const bool isStatic) {
        //isStatic ? static method : special method
        const auto [className, methodName, methodDescriptor] =
                getConstantStringFromPoolByClassNameType(constantPool, index);

        const auto [paramType, returnType] = parseMethodDescriptor(methodDescriptor);
        const auto paramSlotSize = pushParams(blockContext, paramType, !isStatic);

        //pushParams 完后之后 调用helpFunction llvm_compile_invoke_method_fixed去实际调用的函数
        //llvm_compile_invoke_method_fixed中除了调用具体函数还有非常重要的几个工作
        //0. 如果是static 或者 special 调用 则被调函数地址会直接算好写在ir里 直接调用即可
        //如果是对象函数调用 则如要将index传进去 由method_fixed算处具体的调用函数
        //1. 虽然已经通过 pushParams 将参数写入操作数栈 但还没有增加当前栈的sp 会在这个method_fixed中增加
        //2. 在调用完成时 假设有返回值 我们依然可以通过传参同样的方式从当前函数操作数栈里拿到返回值
        //但拿到返回值理应通过pop操作数栈完成 所以在调用完成后method_fixed还需要做返回值对应sp的减操作

        const auto methodRef = klass.getRefMethod(index, isStatic);

        invokeCommon(blockContext, methodName, returnType, getConstantPtr(methodRef), paramSlotSize);
    }

    void MethodCompiler::invokeVirtualMethod(BlockContext &blockContext, const u2 index) {
        const auto [className, methodName, methodDescriptor] =
         getConstantStringFromPoolByClassNameType(constantPool, index);

        const auto [paramType, returnType] = parseMethodDescriptor(methodDescriptor);
        const auto paramSlotSize = pushParams(blockContext, paramType, true);

        if (isMethodHandleInvoke(className, methodName)) {
            const auto invokeMethod =
                    vm.bootstrapClassLoader
                    ->getBasicJavaClass(BasicJavaClassEnum::JAVA_LANG_INVOKE_METHOD_HANDLE)
                    ->getMethod(methodName, METHOD_HANDLE_INVOKE_ORIGIN_DESCRIPTOR, false);

            invokeCommon(blockContext, methodName, returnType, getConstantPtr(invokeMethod), paramSlotSize);
        } else {
            invokeCommon(blockContext, methodName, returnType, getZeroValue(SlotTypeEnum::REF), index);
        }

    }

    void MethodCompiler::invokeDynamic(BlockContext &blockContext, const u2 index) {
        const auto invokeDynamicInfo = CAST_CONSTANT_INVOKE_DYNAMIC_INFO(constantPool[index].get());
        const auto [invokeName, invokeDescriptor] =
                getConstantStringFromPoolByNameAndType(constantPool, invokeDynamicInfo->nameAndTypeIndex);
        const auto [paramType, returnType] = parseMethodDescriptor(invokeDescriptor);
        const auto paramSize = pushParams(blockContext, paramType, false);
        const i4 length = (CAST_U2(paramSize) << 16) | (index & 0xFFFF);

        const auto hasExceptionPtr = irBuilder.CreateAlloca(irBuilder.getInt32Ty());
        irBuilder.CreateStore(irBuilder.getInt32(0), hasExceptionPtr);

        const auto callSiteObj =
                helpFunction->createCallNew(
                    irBuilder,
                    getFramePtr(),
                    LLVM_COMPILER_NEW_DYNAMIC_INVOKE,
                    irBuilder.getInt32(length),
                    getZeroValue(SlotTypeEnum::REF),
                    hasExceptionPtr
                );

        throwNpeIfNull(blockContext, callSiteObj);
        blockContext.pushValue(callSiteObj);
    }

    void MethodCompiler::processInvokeReturn(BlockContext &blockContext, const cview returnType) {
        if (const auto returnSlotType = getSlotTypeByPrimitiveClassName(returnType);
            returnSlotType != SlotTypeEnum::NONE) {
            //如果有返回值 则肯定在当前函数操作数栈的第一位 根据地址关系 借助getLocalVariableTableValue函数拿到返回值
            // const auto returnValue = getLocalVariableTableValue(localCount, returnSlotType);
            const auto returnValue =
                    irBuilder.CreateLoad(slotTypeMap(returnSlotType), getInvokeReturnPtr(blockContext), "invoke_return");
            blockContext.pushValue(returnValue, returnSlotType);
        }
    }

    void MethodCompiler::newOpCode(BlockContext &blockContext, const uint8_t type, llvm::Value *length,
                                   llvm::Value *klass) {
        const auto hasExceptionPtr = irBuilder.CreateAlloca(irBuilder.getInt32Ty());
        irBuilder.CreateStore(irBuilder.getInt32(0), hasExceptionPtr);

        const auto newObject =
                helpFunction->createCallNew(irBuilder, getFramePtr(), type, length, klass, hasExceptionPtr);

        blockContext.pushValue(newObject);
    }

    void MethodCompiler::newObject(BlockContext &blockContext, const u2 index) {
        const auto className = getConstantStringFromPoolByIndexInfo(constantPool, index);
        const auto refClass = klass.classLoader.getClass(className);
        newOpCode(blockContext, LLVM_COMPILER_NEW_OBJECT, irBuilder.getInt32(0), getConstantPtr(refClass));
    }

    void MethodCompiler::newArray(BlockContext &blockContext, const u1 type, llvm::Value *length) {
        newOpCode(blockContext, type, length, getZeroValue(SlotTypeEnum::REF));
    }

    void MethodCompiler::newObjectArray(BlockContext &blockContext, const u2 index, llvm::Value *length) {
        const auto className = getConstantStringFromPoolByIndexInfo(constantPool, index);
        const auto refClass = klass.classLoader.getClass(className);
        const auto arrayClass = klass.classLoader.getObjectArrayClass(*refClass);
        newOpCode(blockContext, LLVM_COMPILER_NEW_OBJECT_ARRAY, length, getConstantPtr(arrayClass));
    }

    void MethodCompiler::newMultiArray(BlockContext &blockContext, const u2 index, const u1 dimension, llvm::Value *arrayDim) {
        const i4 complex = (CAST_U2(dimension) << 16) | (index & 0xFFFF);
        newOpCode(blockContext, LLVM_COMPILER_NEW_MULTI_ARRAY, irBuilder.getInt32(complex), arrayDim);
    }

    void MethodCompiler::checkCast(BlockContext &blockContext, const u2 index, llvm::Value *ref) {
        const auto className = getConstantStringFromPoolByIndexInfo(constantPool, index);
        const auto checkClass = klass.classLoader.getClass(className);

        const auto endBB = BasicBlock::Create(ctx);
        const auto isNotNull = BasicBlock::Create(ctx);
        const auto valIsNull = irBuilder.CreateICmpEQ(ref, getZeroValue(SlotTypeEnum::REF));
        irBuilder.CreateCondBr(valIsNull, endBB, isNotNull);

        changeBB(blockContext, isNotNull);
        const auto checkRet =
                helpFunction->createCallMisc(
                    irBuilder,
                    getFramePtr(),
                    ref,
                    getConstantPtr(checkClass),
                    LLVM_COMPILER_MISC_CHECK_INSTANCE_OF
                );

        const auto notInstanceOfBB = BasicBlock::Create(ctx);
        const auto cmpNotInstanceOf = irBuilder.CreateICmpEQ(checkRet, getZeroValue(SlotTypeEnum::I4));
        irBuilder.CreateCondBr(cmpNotInstanceOf, notInstanceOfBB, endBB);

        changeBB(blockContext, notInstanceOfBB);
        //throw exception
        //return function
        helpFunction->createCallThrowException(
            irBuilder,
            getFramePtr(),
            ref,
            blockContext.pc,
            LLVM_COMPILER_FIXED_EXCEPTION_CLASS_CHECK,
            getConstantPtr(checkClass)
        );

        if (useException) {
            const auto fixedExceptionClass = klass.classLoader.getInstanceClass("java/lang/ClassCastException");
            const auto catchBlocks = cfg.findCatchBlock(blockContext.pc, fixedExceptionClass);
            if (!catchBlocks.empty()) {
                const auto catchBlock = catchBlocks.back();
                const auto blkCtx = cfgBlocks[catchBlock->index].get();
                writeModifyLocalVariableTable(blockContext);
                irBuilder.CreateBr(blkCtx->basicBlock);
            } else {
                exitMethod();
            }
        } else {
            exitMethod();
        }

        changeBB(blockContext, endBB);
    }

    void MethodCompiler::instanceOf(BlockContext &blockContext, const u2 index, llvm::Value *ref) {
        const auto className = getConstantStringFromPoolByIndexInfo(constantPool, index);
        const auto checkClass = klass.classLoader.getClass(className);
        const auto checkRet =
                helpFunction->createCallMisc(
                    irBuilder,
                    getFramePtr(),
                    ref,
                    getConstantPtr(checkClass),
                    LLVM_COMPILER_MISC_CHECK_INSTANCE_OF
                );
        blockContext.pushValue(checkRet);
    }

    void MethodCompiler::monitor(BlockContext &blockContext, const u1 type, llvm::Value *oop) {
        throwNpeIfNull(blockContext, oop);
        helpFunction->createCallMisc(irBuilder, getFramePtr(), oop, getZeroValue(SlotTypeEnum::REF), type);
        // helpFunction->createCallMonitor(irBuilder, oop, type);
    }

    void MethodCompiler::iinc(BlockContext &blockContext, const i4 index, const i4 value) {
        const auto curVal = getLocalVariableTableValue(blockContext, index, SlotTypeEnum::I4);
        const auto incValue = irBuilder.CreateAdd(curVal, irBuilder.getInt32(value));
        setLocalVariableTableValue(blockContext, index, incValue, SlotTypeEnum::I4);
    }

    void MethodCompiler::initClass(BlockContext &blockContext, InstanceClass *klass) {
        const auto invokeException =
                helpFunction->createCallInvokeMethodFixed(
                    irBuilder,
                    getFramePtr(),
                    getConstantPtr(klass),
                    0,
                    blockContext.pc,
                    klass->getClassName(),
                    true
                );

        processCommonException(blockContext, invokeException);
    }

    void MethodCompiler::processCommonException(BlockContext &blockContext, llvm::Value *exception) {
        const auto noExceptionBB = BasicBlock::Create(ctx);
        BasicBlock *exceptionBB = useException ? BasicBlock::Create(ctx) : exitBB;

        const auto noException =
                irBuilder.CreateICmpEQ(
                    exception,
                    getZeroValue(SlotTypeEnum::REF)
                );

        //如果调用中发生了异常 在JIT有处理异常能力之前 先直接退出
        irBuilder.CreateCondBr(noException, noExceptionBB, exceptionBB);

        if (useException) {
            const auto catchBlks = cfg.findCatchBlock(blockContext.pc, nullptr);
            changeBB(blockContext, exceptionBB);
            if (catchBlks.empty()) {
                irBuilder.CreateBr(exitBB);
            } else {
                const auto arrayType = ArrayType::get(voidPtrType, catchBlks.size());
                const auto catchClassArray = irBuilder.CreateAlloca(arrayType);
                for (size_t i = 0; i < catchBlks.size(); ++i) {
                    const auto ptr = irBuilder.CreateGEP(voidPtrType, catchClassArray, irBuilder.getInt32(i));
                    irBuilder.CreateStore(getConstantPtr(catchBlks[i]->catchClass), ptr);
                }
                const auto matchIdx =
                        helpFunction->createCallMatchCatch(
                            irBuilder,
                            exception,
                            catchClassArray,
                            catchBlks.size()
                        );

                writeModifyLocalVariableTable(blockContext);
                const auto switchCatchBB = irBuilder.CreateSwitch(matchIdx, exitBB);
                for (size_t i = 0; i < catchBlks.size(); ++i) {
                    const auto blk = catchBlks[i];
                    const auto blkCtx = cfgBlocks[blk->index].get();
                    switchCatchBB->addCase(irBuilder.getInt32(i), blkCtx->basicBlock);
                }
            }
        }
        changeBB(blockContext, noExceptionBB);
    }


    void MethodCompiler::initCommonBlock() {
        //Entry Block
        const auto firstBlockContext= cfgBlocks[0].get();
        const auto firstBlock = firstBlockContext->basicBlock;
        irBuilder.SetInsertPoint(entryBlock);
        irBuilder.CreateBr(firstBlock);
        //Entry Block End

        //Exit Block
        exitBB->insertInto(function);
        irBuilder.SetInsertPoint(exitBB);
        irBuilder.CreateRetVoid();
        //Exit Block End
    }

    void MethodCompiler::addParamSlot(const BlockContext &blockContext, const u4 paramCount) {
        //把传参的PTR都在entry里初始化完成 后续调用直接使用
        if (invokeMethodParamPtr.size() >= paramCount) {
            return;
        }
        const auto addCount = paramCount - invokeMethodParamPtr.size();
        irBuilder.SetInsertPoint(entryBlock);
        const auto currentCount = invokeMethodParamPtr.size();

        for (size_t i = 0; i < addCount; ++i) {
            const auto realIdx = i +currentCount;
            const auto slotIdx = irBuilder.getInt32(localCount + realIdx);
            const auto paramName = cformat("invoke_param_{}", realIdx);
            const auto paramTypeName = cformat("invoke_paramType_{}", realIdx);

            const auto slotPtr =
                    irBuilder.CreateGEP(irBuilder.getInt64Ty(), getLocalVariableTablePtr(), slotIdx, paramName);
            const auto slotTypePtr =
                    irBuilder.CreateGEP(irBuilder.getInt8Ty(), getLocalVariableTableTypePtr(), slotIdx, paramTypeName);

            invokeMethodParamPtr.emplace_back(slotPtr);
            invokeMethodParamTypePtr.emplace_back(slotTypePtr);
        }

        irBuilder.SetInsertPoint(blockContext.lastBasicBlock);
    }

    llvm::Value * MethodCompiler::getInvokeReturnPtr(const BlockContext &blockContext) const {
        return returnValuePtr;
    }

    llvm::Value *MethodCompiler::loadThrowValue() {
        // helpFunction->createCleanThrow(irBuilder, getFramePtr());
        helpFunction->createCallMisc(
            irBuilder,
            getFramePtr(),
            getZeroValue(SlotTypeEnum::REF),
            getZeroValue(SlotTypeEnum::REF),
            LLVM_COMPILER_MISC_CLEAN_THROW
        );
        return irBuilder.CreateLoad(voidPtrType, getThrowValuePtr());
    }

    bool MethodCompiler::compile() {
        if (cfgBlocks.empty()) {
            panic("error cfgBlock count");
        }

        for (const auto &cfgBlock: cfgBlocks) {
            cfgBlock->compile();
        }

        initCommonBlock();
        return true;
    }

    void MethodCompiler::verify() const {
        for (const auto &cfgBlock: cfgBlocks) {
            if (!cfgBlock->blockValueStack.empty()) {
                panic("error block");
            }
        }

#ifdef DEBUG
        if (verifyFunction(*function, &outs())) {
            cprintlnErr("verify failed method: {} {} {}", klass.getClassName(), method.getName(), method.getDescriptor());
            module.print(llvm::errs(), nullptr);
            panic("exit");
        }
#endif
    }



}
