#include "llvm_compiler.hpp"
#include "llvm_block_context.hpp"
#include "jit_help_function.hpp"
#include "llvm_jit_register_help_function.hpp"
#include "../vm.hpp"
#include "../frame.hpp"
#include "../class.hpp"
#include "../class_member.hpp"
#include "../cfg.hpp"
#include "../class_loader.hpp"
#include "../constant_info.hpp"
#include "../method_handle.hpp"
#include "../oop.hpp"
#include "../utils/descriptor_parser.hpp"


namespace RexVM {
    using namespace llvm;

    MethodCompiler::MethodCompiler(
        VM &vm,
        Method &method,
        Module &module,
        const cview compiledMethodName
    ) : vm(vm),
        method(method),
        klass(method.klass),
        constantPool(klass.constantPool),
        cfg(method),
        module(module),
        ctx(module.getContext()),
        irBuilder(ctx),
        helpFunction(std::make_unique<LLVMHelpFunction>(module)),
        voidPtrType(PointerType::getUnqual(irBuilder.getVoidTy())) {

        const auto functionType =
                FunctionType::get(
                    irBuilder.getVoidTy(),
                    {voidPtrType, voidPtrType, voidPtrType},
                    false
                );

        function = Function::Create(
            functionType,
            GlobalVariable::LinkageTypes::ExternalLinkage,
            compiledMethodName,
            module
        );

        exitBB = BasicBlock::Create(ctx, "exit_method");

        initCFGBlocks();
    }

    void MethodCompiler::initCFGBlocks() {
        cfgBlocks.reserve(cfg.blocks.size());
        for (const auto &cfgBlock : cfg.blocks) {
            const auto lineNumber = method.getLineNumber(cfgBlock->startPC);
            const auto blockName = cformat("{}_{}", cfgBlock->startPC, lineNumber);
            cfgBlocks.emplace_back(std::make_unique<BlockContext>(*this, cfgBlock.get()));
        }

        for (size_t i = 0; i < cfg.blocks.size(); ++i) {
            for (const auto parents = cfg.blocks[i]->parentBlockIndex;
                 const auto parentIdx: parents) {
                cfgBlocks[i]->parentBlocks.emplace_back(cfgBlocks[parentIdx].get());
            }
        }

    }

    void MethodCompiler::startBB(llvm::BasicBlock *nextBasicBlock) {
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
            default:
                panic("error slot type");
            return nullptr;
        } 
    }


    llvm::Value *MethodCompiler::getLocalVariableTableValue(const u4 index, const SlotTypeEnum slotType) {
        //lvt指向Slot[] 先通过GEP获得index对应的Slot指针
        //在解释器中 因为op stack中的也全都是slot 所以不用做转换
        //但在jit中 Value都是具体的类型 所以需要做转换
        //使用createLoad可以自动转换成对应的类型 代码中会生成align信息
        const auto indexValue = irBuilder.getInt32(index);
        const auto type = slotTypeMap(slotType);

        const auto ptr =
                irBuilder.CreateGEP(
                    irBuilder.getInt64Ty(),
                    getLocalVariableTablePtr(),
                    indexValue
                );

        return irBuilder.CreateLoad(type, ptr);
    }

    void MethodCompiler::setLocalVariableTableValue(const u4 index, llvm::Value *value, SlotTypeEnum slotType) {
        // 写lvt和lvtType
        const auto indexValue = irBuilder.getInt32(index);

        const auto lvtPtr =
                irBuilder.CreateGEP(
                    irBuilder.getInt64Ty(),
                    getLocalVariableTablePtr(),
                    indexValue
                );

        const auto lvtTypePtr =
                irBuilder.CreateGEP(
                    irBuilder.getInt8Ty(),
                    getLocalVariableTableTypePtr(),
                    indexValue
                );

        irBuilder.CreateStore(value, lvtPtr);
        irBuilder.CreateStore(irBuilder.getInt8(static_cast<uint8_t>(slotType)), lvtTypePtr);
    }

    llvm::Value * MethodCompiler::getOopDataPtr(llvm::Value *oop, llvm::Value *index, const bool isArray, const BasicType type) {
        const auto offset =
            irBuilder.getInt32(isArray ? ARRAY_OOP_DATA_FIELD_OFFSET : INSTANCE_OOP_DATA_FIELD_OFFSET);

        const auto oopDataFieldPtr =
                irBuilder.CreateGEP(irBuilder.getInt8Ty(), oop, offset);

        const auto dataFieldValue = irBuilder.CreateLoad(voidPtrType, oopDataFieldPtr);
        const u4 elementByteSize = isArray ? getElementSizeByBasicType(type) : SLOT_BYTE_SIZE;
        const u4 elementBitSize = elementByteSize * 8;

        //InstanceOop element is Slot
        const auto dataFieldPtr =
                irBuilder.CreateGEP(
                    irBuilder.getIntNTy(elementBitSize),
                    dataFieldValue,
                    index
                );

        return dataFieldPtr;
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

    void MethodCompiler::throwNpeIfZero(BlockContext &blockContext, llvm::Value *val, const SlotTypeEnum slotType) {
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
        helpFunction->createCallThrowException(
            irBuilder,
            getFramePtr(),
            getZeroValue(SlotTypeEnum::REF),
            blockContext.pc,
            fixedException
        );
        exitMethod();

        changeBB(blockContext, elseBB);
    }


    void MethodCompiler::throwNpeIfNull(BlockContext &blockContext, llvm::Value *val) {
        throwNpeIfZero(blockContext, val, SlotTypeEnum::REF);
    }

    void MethodCompiler::throwException(BlockContext &blockContext, llvm::Value *ex) {
        throwNpeIfNull(blockContext, ex);
        helpFunction->createCallThrowException(irBuilder, getFramePtr(), ex, blockContext.pc, -1);
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
        const auto value = getLocalVariableTableValue(index, slotType);
        blockContext.pushValue(value, slotType);
    }

    void MethodCompiler::store(BlockContext &blockContext, const u4 index, const SlotTypeEnum slotType) {
        const auto value = blockContext.popValue(slotType);
        setLocalVariableTableValue(index, value, slotType); 
    }

    void MethodCompiler::arrayLength(BlockContext &blockContext, llvm::Value *arrayRef) {
        throwNpeIfNull(blockContext, arrayRef);
        const auto arrayLength = helpFunction->createCallArrayLength(irBuilder, arrayRef);
        blockContext.pushValue(arrayLength);
    }

    void MethodCompiler::arrayLoad(BlockContext &blockContext, llvm::Value *arrayRef, llvm::Value *index,
                                   const uint8_t type) {
        //arrayRef 是 arrayOop
        throwNpeIfNull(blockContext, arrayRef);
        switch (type) {
            case LLVM_COMPILER_INT_ARRAY_TYPE: {
                const auto dataPtr = getOopDataPtr(arrayRef, index, true, BasicType::T_INT);
                blockContext.pushValue(irBuilder.CreateLoad(slotTypeMap(SlotTypeEnum::I4), dataPtr));
                break;
            }

            case LLVM_COMPILER_BYTE_ARRAY_TYPE: {
                const auto dataPtr = getOopDataPtr(arrayRef, index, true, BasicType::T_BYTE);
                blockContext.pushValue(irBuilder.CreateLoad(slotTypeMap(SlotTypeEnum::I4), dataPtr));
                break;
            }

            case LLVM_COMPILER_CHAR_ARRAY_TYPE: {
                const auto dataPtr = getOopDataPtr(arrayRef, index, true, BasicType::T_CHAR);
                blockContext.pushValue(irBuilder.CreateLoad(slotTypeMap(SlotTypeEnum::I4), dataPtr));
                break;
            }

            case LLVM_COMPILER_SHORT_ARRAY_TYPE: {
                const auto dataPtr = getOopDataPtr(arrayRef, index, true, BasicType::T_SHORT);
                blockContext.pushValue(irBuilder.CreateLoad(slotTypeMap(SlotTypeEnum::I4), dataPtr));
                break;
            }

            case LLVM_COMPILER_LONG_ARRAY_TYPE: {
                const auto dataPtr = getOopDataPtr(arrayRef, index, true, BasicType::T_LONG);
                blockContext.pushWideValue(irBuilder.CreateLoad(slotTypeMap(SlotTypeEnum::I8), dataPtr));
                break;
            }

            case LLVM_COMPILER_FLOAT_ARRAY_TYPE: {
                const auto dataPtr = getOopDataPtr(arrayRef, index, true, BasicType::T_FLOAT);
                blockContext.pushValue(irBuilder.CreateLoad(slotTypeMap(SlotTypeEnum::F4), dataPtr));
                break;
            }

            case LLVM_COMPILER_DOUBLE_ARRAY_TYPE: {
                const auto dataPtr = getOopDataPtr(arrayRef, index, true, BasicType::T_DOUBLE);
                blockContext.pushWideValue(irBuilder.CreateLoad(slotTypeMap(SlotTypeEnum::F8), dataPtr));
                break;
            }
            case LLVM_COMPILER_OBJ_ARRAY_TYPE: {
                const auto dataPtr = getOopDataPtr(arrayRef, index, true, BasicType::T_OBJECT);
                blockContext.pushValue(irBuilder.CreateLoad(slotTypeMap(SlotTypeEnum::REF), dataPtr));
                break;
            }

            default:
                panic("error type");
        }
    }

    void MethodCompiler::arrayStore(BlockContext &blockContext, llvm::Value *arrayRef, llvm::Value *index, llvm::Value *value, const uint8_t type) {
        throwNpeIfNull(blockContext, arrayRef);
         switch (type) {
            case LLVM_COMPILER_INT_ARRAY_TYPE: {
                const auto dataPtr = getOopDataPtr(arrayRef, index, true, BasicType::T_INT);
                irBuilder.CreateStore(value, dataPtr);
                break;
            }

            case LLVM_COMPILER_BYTE_ARRAY_TYPE: {
                const auto dataPtr = getOopDataPtr(arrayRef, index, true, BasicType::T_BYTE);
                irBuilder.CreateStore(value, dataPtr);
                break;
            }

            case LLVM_COMPILER_CHAR_ARRAY_TYPE: {
                const auto dataPtr = getOopDataPtr(arrayRef, index, true, BasicType::T_CHAR);
                irBuilder.CreateStore(value, dataPtr);
                break;
            }

            case LLVM_COMPILER_SHORT_ARRAY_TYPE: {
                const auto dataPtr = getOopDataPtr(arrayRef, index, true, BasicType::T_SHORT);
                irBuilder.CreateStore(value, dataPtr);
                break;
            }

            case LLVM_COMPILER_LONG_ARRAY_TYPE: {
                const auto dataPtr = getOopDataPtr(arrayRef, index, true, BasicType::T_LONG);
                irBuilder.CreateStore(value, dataPtr);
                break;
            }

            case LLVM_COMPILER_FLOAT_ARRAY_TYPE: {
                const auto dataPtr = getOopDataPtr(arrayRef, index, true, BasicType::T_FLOAT);
                irBuilder.CreateStore(value, dataPtr);
                break;
            }

            case LLVM_COMPILER_DOUBLE_ARRAY_TYPE: {
                const auto dataPtr = getOopDataPtr(arrayRef, index, true, BasicType::T_DOUBLE);
                irBuilder.CreateStore(value, dataPtr);
                break;
            }
            case LLVM_COMPILER_OBJ_ARRAY_TYPE: {
                const auto dataPtr = getOopDataPtr(arrayRef, index, true, BasicType::T_OBJECT);
                irBuilder.CreateStore(value, dataPtr);
                break;
            }

            default:
                panic("error type");
        }
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

    u4 MethodCompiler::offsetToPC(BlockContext &blockContext, const i4 offset) {
        return CAST_U4(CAST_I4(blockContext.pc) + offset);
    }

    void MethodCompiler::ifOp(BlockContext &blockContext, const i4 offset, llvm::Value *val1, llvm::Value *val2, const OpCodeEnum op) {
        const auto jumpTo = offsetToPC(blockContext, offset);
        const auto jumpToBB = getBlockContext(jumpTo)->basicBlock;
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
        const auto elseBB = BasicBlock::Create(ctx);
        irBuilder.CreateCondBr(cmp, jumpToBB, elseBB);
        changeBB(blockContext, elseBB);
    }

    void MethodCompiler::jumpToPC(const u4 pc) {
        const auto jumpToBB = getBlockContext(pc)->basicBlock;
        irBuilder.CreateBr(jumpToBB);
    }

    void MethodCompiler::jumpTo(BlockContext &blockContext, const i4 offset) {
        const auto jumpTo = offsetToPC(blockContext, offset);
        jumpToPC(jumpTo);
    }

    void MethodCompiler::createSwitch(BlockContext &blockContext, llvm::Value *val, std::vector<i4> cases) {
        const auto defaultOffset = cases[0];
        const auto defaultBlock = getBlockContext(offsetToPC(blockContext, defaultOffset))->basicBlock;

        const auto switchIr = irBuilder.CreateSwitch(val, defaultBlock);
        if (cases.size() > 1) {
            for (size_t i = 1; i < cases.size(); i+= 2) {
                const auto caseVal = irBuilder.getInt32(cases[i]);
                const auto caseOffset = cases[i + 1];
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

    std::tuple<Field *, void *> MethodCompiler::getFieldInfo(const u2 index, const bool isStatic) const {
        const auto fieldRef = klass.getRefField(index, isStatic);
        const auto fieldClass = &fieldRef->klass;
        const auto dataPtr = isStatic ? fieldClass->staticData.get() + fieldRef->slotId : nullptr;
        return std::make_tuple(fieldRef, dataPtr);
    }

    void MethodCompiler::getStatic(BlockContext &blockContext, const u2 index) {
        const auto [field, dataPtr] = getFieldInfo(index, true);
        const auto type = field->getFieldSlotType();
        const auto klassPtr = &field->klass;
        helpFunction->createCallClinit(irBuilder, getFramePtr(), getConstantPtr(klassPtr));
        const auto value = irBuilder.CreateLoad(slotTypeMap(type), getConstantPtr(dataPtr));
        blockContext.pushValue(value, type);
    }

    void MethodCompiler::putStatic(BlockContext &blockContext, const u2 index) {
        const auto [field, dataPtr] = getFieldInfo(index, true);
        const auto type = field->getFieldSlotType();
        const auto value = blockContext.popValue(type);
        irBuilder.CreateStore(value, getConstantPtr(dataPtr));
    }

    void MethodCompiler::getField(BlockContext &blockContext, const u2 index) {
        const auto [field, ignored] = getFieldInfo(index, false);
        const auto type = field->getFieldSlotType();
        const auto slotId = field->slotId;
        const auto oop = blockContext.popValue();
        throwNpeIfNull(blockContext, oop);
        const auto fieldDataPtr = getOopDataPtr(oop, irBuilder.getInt32(slotId), false, BasicType::T_OBJECT);
        const auto value = irBuilder.CreateLoad(slotTypeMap(type), fieldDataPtr);
        blockContext.pushValue(value, type);
    }

    void MethodCompiler::putField(BlockContext &blockContext, const u2 index) {
        const auto [field, ignored] = getFieldInfo(index, false);
        const auto type = field->getFieldSlotType();
        const auto slotId = field->slotId;
        const auto value = blockContext.popValue(type);
        const auto oop = blockContext.popValue();
        throwNpeIfNull(blockContext, oop);
        const auto fieldDataPtr = getOopDataPtr(oop, irBuilder.getInt32(slotId), false, BasicType::T_OBJECT);
        irBuilder.CreateStore(value, fieldDataPtr);
    }

    size_t MethodCompiler::pushParams(BlockContext &blockContext, const std::vector<cstring> &paramType, const bool includeThis) {
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

        //根据Frame的初始化定义 operandStackPtr = lvtPtr + lvtSize
        //对于编译后的java函数 lvtSize = method.maxLocals 所以可以直接计算出操作数栈的地址
        //所以 operandStackPtr = lvtPtr + method.maxLocals 它代表第一个栈元素地址
        //后往前依次写入operandStack(std::stack 不能按index读取)
        for (i4 i = CAST_I4(paramSlotType.size()) - 1; i >= 0; --i) {
            //假设有4个参数 则 i = 3,2,1,0
            //具体的参数
            const auto paramValue = blockContext.popValue();
            if (includeThis && i == 0) {
                //first param: this object
                throwNpeIfNull(blockContext, paramValue);
            }

            //lvtPtr + method.paramSlotSize + i 代表了从最后一个到第一个参数的地址
            const auto slotIdx = irBuilder.getInt32(method.maxLocals + i);
            const auto slotPtr =
                    irBuilder.CreateGEP(
                        irBuilder.getInt64Ty(),
                        getLocalVariableTablePtr(),
                        slotIdx
                    );

            const auto slotTypePtr =
                    irBuilder.CreateGEP(
                        irBuilder.getInt8Ty(),
                        getLocalVariableTableTypePtr(),
                        slotIdx
                    );

            if (paramValue != nullptr) {
                //nullptr 是padding
                irBuilder.CreateStore(paramValue, slotPtr);
            }

            const auto slotType = paramSlotType[i];
            irBuilder.CreateStore(irBuilder.getInt8(static_cast<uint8_t>(slotType)), slotTypePtr);
        }
        return paramSlotType.size();
    }

    void MethodCompiler::invokeStaticMethod(BlockContext &blockContext, const u2 index, const bool isStatic) {
        //isStatic ? static method : special method
        const auto [className, methodName, methodDescriptor] =
                getConstantStringFromPoolByClassNameType(constantPool, index);

        const auto [paramType, returnType] = parseMethodDescriptor(methodDescriptor);
        const auto paramSlotSize = pushParams(blockContext, paramType, !isStatic);

        const auto methodRef = klass.getRefMethod(index, isStatic);
        helpFunction->createCallInvokeMethodFixed(irBuilder, getFramePtr(), getConstantPtr(methodRef), paramSlotSize);
        processInvokeReturn(blockContext, returnType);
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

            helpFunction->createCallInvokeMethodFixed(
                irBuilder,
                getFramePtr(),
                getConstantPtr(invokeMethod),
                paramSlotSize
            );
            return;
        }

        //Add sp and invoke method
        helpFunction->createCallInvokeMethodFixed(irBuilder, getFramePtr(), getZeroValue(SlotTypeEnum::REF), index);
        processInvokeReturn(blockContext, returnType);
    }

    void MethodCompiler::invokeDynamic(BlockContext &blockContext, const u2 index) {
        const auto invokeDynamicInfo = CAST_CONSTANT_INVOKE_DYNAMIC_INFO(constantPool[index].get());
        const auto [invokeName, invokeDescriptor] =
                getConstantStringFromPoolByNameAndType(constantPool, invokeDynamicInfo->nameAndTypeIndex);
        const auto [paramType, returnType] = parseMethodDescriptor(invokeDescriptor);
        const auto paramSize = pushParams(blockContext, paramType, false);
        const i4 length = (CAST_U2(paramSize) << 16) | (index & 0xFFFF);

        const auto callSiteObj =
                helpFunction->createCallNew(
                    irBuilder,
                    getFramePtr(),
                    LLVM_COMPILER_NEW_DYNAMIC_INVOKE,
                    irBuilder.getInt32(length),
                    getZeroValue(SlotTypeEnum::REF)
                );
        throwNpeIfNull(blockContext, callSiteObj);
        blockContext.pushValue(callSiteObj);
    }

    void MethodCompiler::processInvokeReturn(BlockContext &blockContext, const cview returnType) {
        if (const auto returnSlotType = getSlotTypeByPrimitiveClassName(returnType);
            returnSlotType != SlotTypeEnum::NONE) {
            //has return value
            const auto returnValue = getLocalVariableTableValue(method.maxLocals, returnSlotType);
            blockContext.pushValue(returnValue, returnSlotType);
        }
    }

    void MethodCompiler::newOpCode(BlockContext &blockContext, const uint8_t type, llvm::Value *length,
                                   llvm::Value *klass) {
        const auto newObject = helpFunction->createCallNew(irBuilder, getFramePtr(), type, length, klass);
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

        const auto checkRet =
                helpFunction->createCallInstanceOf(
                    irBuilder,
                    getFramePtr(),
                    LLVM_COMPILER_CHECK_CAST,
                    ref,
                    getConstantPtr(checkClass)
                );

        const auto endBB = BasicBlock::Create(ctx);
        const auto notInstanceOfBB = BasicBlock::Create(ctx);
        const auto cmpNotInstanceOf = irBuilder.CreateICmpEQ(checkRet, getZeroValue(SlotTypeEnum::I4));
        irBuilder.CreateCondBr(cmpNotInstanceOf, notInstanceOfBB, endBB);

        changeBB(blockContext, notInstanceOfBB);
        //already throw exception in createCallInstanceOf
        //return function
        exitMethod();

        changeBB(blockContext, endBB);
    }

    void MethodCompiler::instanceOf(BlockContext &blockContext, const u2 index, llvm::Value *ref) {
        const auto className = getConstantStringFromPoolByIndexInfo(constantPool, index);
        const auto checkClass = klass.classLoader.getClass(className);

        const auto checkRet =
                helpFunction->createCallInstanceOf(
                    irBuilder,
                    getFramePtr(),
                    LLVM_COMPILER_CHECK_CAST,
                    ref,
                    getConstantPtr(checkClass)
                );
        blockContext.pushValue(checkRet);
    }

    void MethodCompiler::monitor(BlockContext &blockContext, const u1 type, llvm::Value *oop) {
        throwNpeIfNull(blockContext, oop);
        helpFunction->createCallMonitor(irBuilder, oop, type);
    }



    void MethodCompiler::compile() {
        for (const auto &cfgBlock: cfgBlocks) {
            cfgBlock->compile();
        }

        exitBB->insertInto(function);
        irBuilder.SetInsertPoint(exitBB);
        irBuilder.CreateRetVoid();

        // ByteReader reader{};
        // const auto codePtr = method.code.get();
        // reader.init(codePtr, method.codeLength);
        //
        //
        // while (!reader.eof()) {
        //     const auto currentByteCode = reader.readU1();
        //     const auto opCode = static_cast<OpCodeEnum>(currentByteCode);
        //     pc = CAST_U4(reader.ptr - reader.begin) - 1;
        //     processInstruction(opCode, reader);
        //     reader.resetCurrentOffset();
        // }
        //
        // if (!valueStack.empty()) {
        //     panic("error value stack");
        // }
        //
        // // irBuilder.CreateBr(returnBB);
        //
        // changeBB(blockContext, returnBB);
        // irBuilder.CreateRetVoid();
        //
        // verifyFunction(*function);

    }

    void MethodCompiler::verify() const {
        for (const auto &cfgBlock: cfgBlocks) {
            if (!cfgBlock->blockValueStack.empty()) {
                panic("error block");
            }
        }

        if (verifyFunction(*function, &outs())) {
            cprintlnErr("verify failed method: {} {} {}", klass.getClassName(), method.getName(), method.getDescriptor());
            module.print(llvm::errs(), nullptr);
            panic("exit");
        }
    }



}
