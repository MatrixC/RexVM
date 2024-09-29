#include "llvm_compiler.hpp"
#include <set>
#include "jit_help_function.hpp"
#include "llvm_jit_register_help_function.hpp"
#include "../frame.hpp"
#include "../class.hpp"
#include "../class_member.hpp"
#include "../constant_info.hpp"
#include "../utils/method_utils.hpp"


namespace RexVM {
    using namespace llvm;

    MethodCompiler::MethodCompiler(
        Method &method,
        Module &module,
        cview compiledMethodName
    ) : method(method),
        klass(method.klass),
        constantPool(klass.constantPool),
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

        initBasicBlock();
        initHelpFunction();
    }

    void MethodCompiler::initBasicBlock() {
        returnBB = BasicBlock::Create(ctx);

        const auto entryBB = BasicBlock::Create(ctx, "entry");
        changeBB(entryBB);
        for (const auto methodLabels = scanMethodLabel(&method);
             const auto &[pc, offset]: methodLabels
        ) {
            auto opCodeBB = BasicBlock::Create(ctx);
            opCodeBlocks.emplace(offset, opCodeBB);
        }
    }

    void MethodCompiler::initHelpFunction() {
        arrayStoreI4 = module.getOrInsertFunction("llvm_compile_array_store_i4",
                                                  FunctionType::get(irBuilder.getVoidTy(), {
                                                                        voidPtrType, irBuilder.getInt8Ty(),
                                                                        irBuilder.getInt32Ty(), irBuilder.getInt32Ty()
                                                                    }, false));
        arrayStoreI8 = module.getOrInsertFunction("llvm_compile_array_store_i8",
                                                  FunctionType::get(irBuilder.getVoidTy(), {
                                                                        voidPtrType, irBuilder.getInt32Ty(),
                                                                        irBuilder.getInt64Ty()
                                                                    }, false));
        arrayStoreF4 = module.getOrInsertFunction("llvm_compile_array_store_f4",
                                                  FunctionType::get(irBuilder.getVoidTy(), {
                                                                        voidPtrType, irBuilder.getInt32Ty(),
                                                                        irBuilder.getFloatTy()
                                                                    }, false));
        arrayStoreF8 = module.getOrInsertFunction("llvm_compile_array_store_f8",
                                                  FunctionType::get(irBuilder.getVoidTy(), {
                                                                        voidPtrType, irBuilder.getInt32Ty(),
                                                                        irBuilder.getDoubleTy()
                                                                    }, false));
        arrayStoreObj = module.getOrInsertFunction("llvm_compile_array_store_obj",
                                                   FunctionType::get(irBuilder.getVoidTy(), {
                                                                         voidPtrType, irBuilder.getInt32Ty(),
                                                                         voidPtrType
                                                                     }, false));

        returnI4 = module.getOrInsertFunction("llvm_compile_return_i4",
                                                   FunctionType::get(irBuilder.getVoidTy(), {
                                                                         voidPtrType, irBuilder.getInt32Ty()
                                                                     }, false));

        returnI8 = module.getOrInsertFunction("llvm_compile_return_i8",
                                           FunctionType::get(irBuilder.getVoidTy(), {
                                                                 voidPtrType, irBuilder.getInt64Ty()
                                                             }, false));

        returnF4 = module.getOrInsertFunction("llvm_compile_return_f4",
                                           FunctionType::get(irBuilder.getVoidTy(), {
                                                                 voidPtrType, irBuilder.getFloatTy()
                                                             }, false));

        returnF8 = module.getOrInsertFunction("llvm_compile_return_f8",
                                           FunctionType::get(irBuilder.getVoidTy(), {
                                                                 voidPtrType, irBuilder.getDoubleTy()
                                                             }, false));

        returnObj = module.getOrInsertFunction("llvm_compile_return_obj",
                                           FunctionType::get(irBuilder.getVoidTy(), {
                                                                 voidPtrType, voidPtrType
                                                             }, false));

        returnVoid = module.getOrInsertFunction("llvm_compile_return_void",
                                           FunctionType::get(irBuilder.getVoidTy(), {
                                                                 voidPtrType
                                                             }, false));
    }

    void MethodCompiler::changeBB(llvm::BasicBlock *nextBasicBlock) {
        if (currentBB != nullptr) {
            if (const auto &lastInstr = currentBB->back(); !lastInstr.isTerminator()) {
                irBuilder.CreateBr(nextBasicBlock);
            }
        }
        currentBB = nextBasicBlock;
        nextBasicBlock->insertInto(function);
        irBuilder.SetInsertPoint(nextBasicBlock);
    }


    llvm::Argument *MethodCompiler::getFramePtr() const {
        return function->arg_begin();
    }

    llvm::Argument *MethodCompiler::getLocalVariableTablePtr() const {
        return function->arg_begin() + 1;
    }

    llvm::Argument *MethodCompiler::getLocalVariableTableTypePtr() const {
        return function->arg_begin() + 2;
    }

    llvm::Type *MethodCompiler::slotTypeMap(SlotTypeEnum slotType) {
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

    llvm::Value *MethodCompiler::getLocalVariableTableValue(u4 index, SlotTypeEnum slotType) {
        //lvt指向Slot[] 先通过getelementptr获得index对应的Slot指针
        //在解释器中 因为op stack中的也全都是slot 所以不用做转换
        //但在jit中 Value都是具体的类型 所以需要做转换
        //使用createLoad可以自动转换成对应的类型 代码中会生成align信息
        Type *type = slotTypeMap(slotType);

        const auto ptr =
                irBuilder.CreateGEP(
                    irBuilder.getInt64Ty(),
                    getLocalVariableTablePtr(),
                    irBuilder.getInt32(index)
                );

        return irBuilder.CreateLoad(type, ptr);

        // llvm::Value *value = irBuilder.CreateLoad(irBuilder.getInt64Ty(), ptr);
        // if (type->getScalarSizeInBits() == 32) {
        //     value = irBuilder.CreateAnd(value, irBuilder.getInt64(0x0F));
        // }
        // return irBuilder.CreateBitCast(value, type);
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


    void MethodCompiler::pushValue(llvm::Value *value) {
        valueStack.emplace(value);
    }

    llvm::Value *MethodCompiler::popValue() {
        const auto value = valueStack.top();
        valueStack.pop();
        return value;
    }

    llvm::Value *MethodCompiler::topValue() {
        return valueStack.top();
    }

    void MethodCompiler::pushI8Padding() {
        pushValue(irBuilder.getInt64(0));
    }

    void MethodCompiler::pushF8Padding() {
        pushValue(ConstantFP::getZero(irBuilder.getDoubleTy()));
    }

    void MethodCompiler::pushI4Const(const i4 value) {
        pushValue(irBuilder.getInt32(static_cast<uint32_t>(value)));
    }

    void MethodCompiler::pushI8Const(const i8 value) {
        pushValue(irBuilder.getInt64(static_cast<uint64_t>(value)));
        pushI8Padding();
    }

    void MethodCompiler::pushF4Const(const f4 value) {
        pushValue(ConstantFP::get(irBuilder.getFloatTy(), value));
    }

    void MethodCompiler::pushF8Const(const f8 value) {
        pushValue(ConstantFP::get(irBuilder.getDoubleTy(), value));
        pushF8Padding();
    }

    void MethodCompiler::pushNullConst() {
        pushValue(ConstantPointerNull::get(voidPtrType));
    }

    void MethodCompiler::throwNpeIfNull(llvm::Value *val) {
        // if (value == nullptr) {
        //   throwNpe(frame);
        //   return;
        // }

        const auto isNullBB = BasicBlock::Create(ctx);
        const auto elseBB = BasicBlock::Create(ctx);

        const auto cmpIsNull = irBuilder.CreateICmpEQ(val, ConstantPointerNull::get(voidPtrType));
        irBuilder.CreateCondBr(cmpIsNull, isNullBB, elseBB);

        changeBB(isNullBB);
        helpFunction->createCallThrowNPE(irBuilder, getFramePtr());
        irBuilder.CreateBr(returnBB);

        changeBB(elseBB);
    }

    void MethodCompiler::ldc_helper(u2 index) {
        const auto valPtr = constantPool[index].get();
        const auto constantTagEnum = CAST_CONSTANT_TAG_ENUM(valPtr->tag);

        switch (constantTagEnum) {
            case ConstantTagEnum::CONSTANT_Integer:
                pushI4Const((CAST_CONSTANT_INTEGER_INFO(valPtr))->value);
                break;

            case ConstantTagEnum::CONSTANT_Long:
                pushI8Const((CAST_CONSTANT_LONG_INFO(valPtr))->value);
                break;

            case ConstantTagEnum::CONSTANT_Float:
                pushF4Const((CAST_CONSTANT_FLOAT_INFO(valPtr))->value);
                break;

            case ConstantTagEnum::CONSTANT_Double:
                pushF8Const((CAST_CONSTANT_DOUBLE_INFO(valPtr))->value);
                break;

            case ConstantTagEnum::CONSTANT_String: {
                pushValue(helpFunction->createCallGetStringConstant(irBuilder, getFramePtr(), index));
                break;
            }

            case ConstantTagEnum::CONSTANT_Class: {
                pushValue(helpFunction->createCallGetClassMirrorConstant(irBuilder, getFramePtr(), index));
                break;
            }

            default:
                panic(cformat("ldc error tag{}", valPtr->tag));
        }
    }

    void MethodCompiler::load(const u4 index, const SlotTypeEnum slotType) {
        switch (slotType) {
            case SlotTypeEnum::I4:
            case SlotTypeEnum::F4:
            case SlotTypeEnum::REF:
                pushValue(getLocalVariableTableValue(index, slotType));
                break;

            case SlotTypeEnum::I8:
                pushValue(getLocalVariableTableValue(index, slotType));
                pushI8Padding();
                break;

            case SlotTypeEnum::F8:
                pushValue(getLocalVariableTableValue(index, slotType));
                pushF8Padding();
                break;

            default:
                panic("error type");
        }
    }

    void MethodCompiler::store(const u4 index, const SlotTypeEnum slotType) {
        switch (slotType) {
            case SlotTypeEnum::I4:
            case SlotTypeEnum::F4:
            case SlotTypeEnum::REF:
                setLocalVariableTableValue(index, popValue(), slotType);
                break;

            case SlotTypeEnum::I8:
            case SlotTypeEnum::F8:
                popValue(); //ZERO
                setLocalVariableTableValue(index, popValue(), slotType);
                break;

            default:
                panic("error type");
        }
    }


    void MethodCompiler::arrayLoad(llvm::Value *arrayRef, llvm::Value *index, uint8_t type) {
        //arrayRef 是 arrayOop
        throwNpeIfNull(arrayRef);
        switch (type) {
            case LLVM_COMPILER_INT_ARRAY_TYPE:
            case LLVM_COMPILER_BYTE_ARRAY_TYPE:
            case LLVM_COMPILER_CHAR_ARRAY_TYPE:
            case LLVM_COMPILER_SHORT_ARRAY_TYPE:
                pushValue(helpFunction->createCallArrayLoadI4(irBuilder, arrayRef, index, type));
                break;

            case LLVM_COMPILER_LONG_ARRAY_TYPE:
                pushValue(helpFunction->createCallArrayLoadI8(irBuilder, arrayRef, index));
                pushI8Padding();
                break;

            case LLVM_COMPILER_FLOAT_ARRAY_TYPE:
                pushValue(helpFunction->createCallArrayLoadF4(irBuilder, arrayRef, index));
                break;

            case LLVM_COMPILER_DOUBLE_ARRAY_TYPE:
                pushValue(helpFunction->createCallArrayLoadF8(irBuilder, arrayRef, index));
                pushF8Padding();
                break;

            case LLVM_COMPILER_OBJ_ARRAY_TYPE:
                pushValue(helpFunction->createCallArrayLoadObj(irBuilder, arrayRef, index));
                break;

            default:
                panic("error type");
        }
    }

    void MethodCompiler::arrayStore(llvm::Value *arrayRef, llvm::Value *index, llvm::Value *value, uint8_t type) {
        throwNpeIfNull(arrayRef);
        switch (type) {
            case LLVM_COMPILER_INT_ARRAY_TYPE:
            case LLVM_COMPILER_BYTE_ARRAY_TYPE:
            case LLVM_COMPILER_CHAR_ARRAY_TYPE:
            case LLVM_COMPILER_SHORT_ARRAY_TYPE:
                irBuilder.CreateCall(arrayStoreI4, {arrayRef, irBuilder.getInt8(type), index, value});
                break;

            case LLVM_COMPILER_LONG_ARRAY_TYPE:
                popValue();
                irBuilder.CreateCall(arrayStoreI8, {arrayRef, index, value});
                break;

            case LLVM_COMPILER_FLOAT_ARRAY_TYPE:
                irBuilder.CreateCall(arrayStoreF4, {arrayRef, index, value});
                break;

            case LLVM_COMPILER_DOUBLE_ARRAY_TYPE:
                popValue();
                irBuilder.CreateCall(arrayStoreF8, {arrayRef, index, value});
                break;

            case LLVM_COMPILER_OBJ_ARRAY_TYPE:
                irBuilder.CreateCall(arrayStoreObj, {arrayRef, index, value});
                break;

            default:
                panic("error type");
        }
    }

    void MethodCompiler::lCmp(llvm::Value *val1, llvm::Value *val2) {
        const auto ifGt = BasicBlock::Create(ctx);
        const auto ifElse = BasicBlock::Create(ctx);
        const auto ifEq = BasicBlock::Create(ctx);
        const auto ifEqElse = BasicBlock::Create(ctx);
        const auto ifEnd = BasicBlock::Create(ctx);

        const auto cmpGt = irBuilder.CreateICmpSGT(val1, val2);
        irBuilder.CreateCondBr(cmpGt, ifGt, ifElse);

        changeBB(ifGt);
        irBuilder.CreateBr(ifEnd);

        changeBB(ifElse);
        const auto cmpEq = irBuilder.CreateICmpEQ(val1, val2);
        irBuilder.CreateCondBr(cmpEq, ifEq, ifEqElse);

        changeBB(ifEq);
        irBuilder.CreateBr(ifEnd);

        changeBB(ifEqElse);
        irBuilder.CreateBr(ifEnd);

        changeBB(ifEnd);
        const auto phi = irBuilder.CreatePHI(irBuilder.getInt32Ty(), 3);
        phi->addIncoming(irBuilder.getInt32(1), ifGt);
        phi->addIncoming(irBuilder.getInt32(0), ifEq);
        phi->addIncoming(irBuilder.getInt32(-1), ifEqElse);

        pushValue(phi);
    }

    void MethodCompiler::fCmp(llvm::Value *val1, llvm::Value *val2, llvm::Value *nanRet) {
        const auto ifGt = BasicBlock::Create(ctx);
        const auto ifGtElse = BasicBlock::Create(ctx);
        const auto ifEq = BasicBlock::Create(ctx);
        const auto ifEqElse = BasicBlock::Create(ctx);
        const auto ifLt = BasicBlock::Create(ctx);
        const auto ifNan = BasicBlock::Create(ctx);
        const auto ifEnd = BasicBlock::Create(ctx);

        const auto cmpGt = irBuilder.CreateFCmpOGT(val1, val2);
        irBuilder.CreateCondBr(cmpGt, ifGt, ifGtElse);

        changeBB(ifGt);
        irBuilder.CreateBr(ifEnd);

        changeBB(ifGtElse);
        const auto cmpEq = irBuilder.CreateFCmpOEQ(val1, val2);
        irBuilder.CreateCondBr(cmpEq, ifEq, ifEqElse);

        changeBB(ifEq);
        irBuilder.CreateBr(ifEnd);

        changeBB(ifEqElse);
        const auto cmpLt = irBuilder.CreateFCmpOLT(val1, val2);
        irBuilder.CreateCondBr(cmpLt, ifLt, ifNan);

        changeBB(ifLt);
        irBuilder.CreateBr(ifEnd);

        changeBB(ifNan);
        irBuilder.CreateBr(ifEnd);

        changeBB(ifEnd);
        const auto phi = irBuilder.CreatePHI(irBuilder.getInt32Ty(), 3);
        phi->addIncoming(irBuilder.getInt32(1), ifGt);
        phi->addIncoming(irBuilder.getInt32(0), ifEq);
        phi->addIncoming(irBuilder.getInt32(-1), ifLt);
        phi->addIncoming(nanRet, ifNan);

        pushValue(phi);
    }

    void MethodCompiler::ifOp(const u4 jumpTo, llvm::Value *val1, llvm::Value *val2, const OpCodeEnum op) {
        const auto offsetBB = opCodeBlocks[jumpTo];
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
        irBuilder.CreateCondBr(cmp, offsetBB, elseBB);
        changeBB(elseBB);
    }



    void MethodCompiler::processInstruction(
        OpCodeEnum opCode,
        u4 pc,
        ByteReader &byteReader
    ) {
        if (const auto iter = opCodeBlocks.find(pc); iter != opCodeBlocks.end()) {
            changeBB(iter->second);
        }

        switch (opCode) {
            case OpCodeEnum::NOP:
                break;
            case OpCodeEnum::ACONST_NULL:
                pushNullConst();
                break;

            case OpCodeEnum::ICONST_M1:
            case OpCodeEnum::ICONST_0:
            case OpCodeEnum::ICONST_1:
            case OpCodeEnum::ICONST_2:
            case OpCodeEnum::ICONST_3:
            case OpCodeEnum::ICONST_4:
            case OpCodeEnum::ICONST_5:
                pushI4Const(CAST_I4(opCode) - CAST_I4(OpCodeEnum::ICONST_0));
                break;
            case OpCodeEnum::LCONST_0:
            case OpCodeEnum::LCONST_1:
                pushI8Const(CAST_I8(opCode) - CAST_I8(OpCodeEnum::LCONST_0));
                break;
            case OpCodeEnum::FCONST_0:
                pushF4Const(0);
                break;
            case OpCodeEnum::FCONST_1:
                pushF4Const(1);
                break;
            case OpCodeEnum::FCONST_2:
                pushF4Const(2);
                break;
            case OpCodeEnum::DCONST_0:
                pushF8Const(0);
                break;
            case OpCodeEnum::DCONST_1:
                pushF8Const(1);
                break;

            case OpCodeEnum::BIPUSH:
                pushI4Const(byteReader.readI1());
                break;
            case OpCodeEnum::SIPUSH:
                pushI4Const(byteReader.readI2());
                break;

            case OpCodeEnum::LDC: {
                const auto index = byteReader.readU1();
                ldc_helper(index);
                break;
            }

            case OpCodeEnum::LDC_W:
            case OpCodeEnum::LDC2_W: {
                const auto index = byteReader.readU2();
                ldc_helper(index);
                break;
            }

            case OpCodeEnum::ILOAD: {
                const auto index = byteReader.readU1();
                load(index, SlotTypeEnum::I4);
                break;
            }
            case OpCodeEnum::LLOAD: {
                const auto index = byteReader.readU1();
                load(index, SlotTypeEnum::I8);
                break;
            }
            case OpCodeEnum::FLOAD: {
                const auto index = byteReader.readU1();
                load(index, SlotTypeEnum::F4);
                break;
            }
            case OpCodeEnum::DLOAD: {
                const auto index = byteReader.readU1();
                load(index, SlotTypeEnum::F8);
                break;
            }
            case OpCodeEnum::ALOAD: {
                const auto index = byteReader.readU1();
                load(index, SlotTypeEnum::REF);
                break;
            }

            case OpCodeEnum::ILOAD_0:
            case OpCodeEnum::ILOAD_1:
            case OpCodeEnum::ILOAD_2:
            case OpCodeEnum::ILOAD_3: {
                const auto idx = CAST_I4(opCode) - CAST_I4(OpCodeEnum::ILOAD_0);
                load(idx, SlotTypeEnum::I4);
                break;
            }

            case OpCodeEnum::LLOAD_0:
            case OpCodeEnum::LLOAD_1:
            case OpCodeEnum::LLOAD_2:
            case OpCodeEnum::LLOAD_3: {
                const auto idx = CAST_I4(opCode) - CAST_I4(OpCodeEnum::LLOAD_0);
                load(idx, SlotTypeEnum::I8);
                break;
            }

            case OpCodeEnum::FLOAD_0:
            case OpCodeEnum::FLOAD_1:
            case OpCodeEnum::FLOAD_2:
            case OpCodeEnum::FLOAD_3: {
                const auto idx = CAST_I4(opCode) - CAST_I4(OpCodeEnum::FLOAD_0);
                load(idx, SlotTypeEnum::F4);
                break;
            }

            case OpCodeEnum::DLOAD_0:
            case OpCodeEnum::DLOAD_1:
            case OpCodeEnum::DLOAD_2:
            case OpCodeEnum::DLOAD_3: {
                const auto idx = CAST_I4(opCode) - CAST_I4(OpCodeEnum::DLOAD_0);
                load(idx, SlotTypeEnum::F8);
                break;
            }

            case OpCodeEnum::ALOAD_0:
            case OpCodeEnum::ALOAD_1:
            case OpCodeEnum::ALOAD_2:
            case OpCodeEnum::ALOAD_3: {
                const auto idx = CAST_I4(opCode) - CAST_I4(OpCodeEnum::ALOAD_0);
                load(idx, SlotTypeEnum::REF);
                break;
            }

            case OpCodeEnum::IALOAD:
            case OpCodeEnum::LALOAD:
            case OpCodeEnum::FALOAD:
            case OpCodeEnum::DALOAD:
            case OpCodeEnum::AALOAD:
            case OpCodeEnum::BALOAD:
            case OpCodeEnum::CALOAD:
            case OpCodeEnum::SALOAD: {
                uint8_t arrayType = CAST_U1(opCode) - CAST_U1(OpCodeEnum::IALOAD);
                const auto idx = popValue();
                const auto arrayRef = popValue();
                arrayLoad(arrayRef, idx, arrayType);
                break;
            }

            case OpCodeEnum::ISTORE: {
                const auto index = byteReader.readU1();
                store(index, SlotTypeEnum::I4);
                break;
            }
            case OpCodeEnum::LSTORE: {
                const auto index = byteReader.readU1();
                store(index, SlotTypeEnum::I8);
                break;
            }
            case OpCodeEnum::FSTORE: {
                const auto index = byteReader.readU1();
                store(index, SlotTypeEnum::F4);
                break;
            }
            case OpCodeEnum::DSTORE: {
                const auto index = byteReader.readU1();
                store(index, SlotTypeEnum::F8);
                break;
            }
            case OpCodeEnum::ASTORE: {
                const auto index = byteReader.readU1();
                store(index, SlotTypeEnum::REF);
                break;
            }

            case OpCodeEnum::ISTORE_0:
            case OpCodeEnum::ISTORE_1:
            case OpCodeEnum::ISTORE_2:
            case OpCodeEnum::ISTORE_3: {
                const auto idx = CAST_I4(opCode) - CAST_I4(OpCodeEnum::ISTORE_0);
                store(idx, SlotTypeEnum::I4);
                break;
            }

            case OpCodeEnum::LSTORE_0:
            case OpCodeEnum::LSTORE_1:
            case OpCodeEnum::LSTORE_2:
            case OpCodeEnum::LSTORE_3: {
                const auto idx = CAST_I4(opCode) - CAST_I4(OpCodeEnum::LSTORE_0);
                store(idx, SlotTypeEnum::I8);
                break;
            }

            case OpCodeEnum::FSTORE_0:
            case OpCodeEnum::FSTORE_1:
            case OpCodeEnum::FSTORE_2:
            case OpCodeEnum::FSTORE_3: {
                const auto idx = CAST_I4(opCode) - CAST_I4(OpCodeEnum::FSTORE_0);
                store(idx, SlotTypeEnum::F4);
                break;
            }

            case OpCodeEnum::DSTORE_0:
            case OpCodeEnum::DSTORE_1:
            case OpCodeEnum::DSTORE_2:
            case OpCodeEnum::DSTORE_3: {
                const auto idx = CAST_I4(opCode) - CAST_I4(OpCodeEnum::DSTORE_0);
                store(idx, SlotTypeEnum::F8);
                break;
            }

            case OpCodeEnum::ASTORE_0:
            case OpCodeEnum::ASTORE_1:
            case OpCodeEnum::ASTORE_2:
            case OpCodeEnum::ASTORE_3: {
                const auto idx = CAST_I4(opCode) - CAST_I4(OpCodeEnum::ASTORE_0);
                store(idx, SlotTypeEnum::REF);
                break;
            }

            case OpCodeEnum::IASTORE:
            case OpCodeEnum::LASTORE:
            case OpCodeEnum::FASTORE:
            case OpCodeEnum::DASTORE:
            case OpCodeEnum::AASTORE:
            case OpCodeEnum::BASTORE:
            case OpCodeEnum::CASTORE:
            case OpCodeEnum::SASTORE: {
                uint8_t arrayType = CAST_U1(opCode) - CAST_U1(OpCodeEnum::IASTORE);
                const auto val = popValue();
                const auto idx = popValue();
                const auto arrayRef = popValue();
                arrayStore(arrayRef, idx, val, arrayType);
                break;
            }

            case OpCodeEnum::POP:
                popValue();
                break;
            case OpCodeEnum::POP2:
                popValue();
                popValue();
                break;
            case OpCodeEnum::DUP:
                pushValue(topValue());
                break;
            case OpCodeEnum::DUP_X1: {
                const auto val1 = popValue();
                const auto val2 = popValue();
                pushValue(val1);
                pushValue(val2);
                pushValue(val1);
                break;
            }

            case OpCodeEnum::DUP_X2: {
                const auto val1 = popValue();
                const auto val2 = popValue();
                const auto val3 = popValue();
                pushValue(val1);
                pushValue(val3);
                pushValue(val2);
                pushValue(val1);
                break;
            }
            case OpCodeEnum::DUP2: {
                const auto val1 = popValue();
                const auto val2 = popValue();
                pushValue(val2);
                pushValue(val1);
                pushValue(val2);
                pushValue(val1);
                break;
            }
            case OpCodeEnum::DUP2_X1: {
                const auto val1 = popValue();
                const auto val2 = popValue();
                const auto val3 = popValue();
                pushValue(val2);
                pushValue(val1);
                pushValue(val3);
                pushValue(val2);
                pushValue(val1);
                break;
            }
            case OpCodeEnum::DUP2_X2: {
                const auto val1 = popValue();
                const auto val2 = popValue();
                const auto val3 = popValue();
                const auto val4 = popValue();
                pushValue(val2);
                pushValue(val1);
                pushValue(val4);
                pushValue(val3);
                pushValue(val2);
                pushValue(val1);
                break;
            }
            case OpCodeEnum::SWAP: {
                const auto val1 = popValue();
                const auto val2 = popValue();
                pushValue(val1);
                pushValue(val2);
                break;
            }

            case OpCodeEnum::IADD:{
                const auto val2 = popValue();
                const auto val1 = popValue();
                pushValue(irBuilder.CreateAdd(val1, val2));
                break;
            }
            case OpCodeEnum::LADD: {
                popValue();
                const auto val2 = popValue();
                popValue();
                const auto val1 = popValue();
                pushValue(irBuilder.CreateAdd(val1, val2));
                pushI8Padding();
                break;
            }
            case OpCodeEnum::FADD:  {
                const auto val2 = popValue();
                const auto val1 = popValue();
                pushValue(irBuilder.CreateFAdd(val1, val2));
                break;
            }
            case OpCodeEnum::DADD: {
                popValue();
                const auto val2 = popValue();
                popValue();
                const auto val1 = popValue();
                pushValue(irBuilder.CreateFAdd(val1, val2));
                pushF8Padding();
                break;
            }

            case OpCodeEnum::ISUB:
            case OpCodeEnum::LSUB: {
                const auto val2 = popValue();
                const auto val1 = popValue();
                pushValue(irBuilder.CreateSub(val1, val2));
                break;
            }

            case OpCodeEnum::FSUB:
            case OpCodeEnum::DSUB: {
                const auto val2 = popValue();
                const auto val1 = popValue();
                pushValue(irBuilder.CreateFSub(val1, val2));
                break;
            }

            case OpCodeEnum::IMUL: {
                const auto val2 = popValue();
                const auto val1 = popValue();
                pushValue(irBuilder.CreateMul(val1, val2));
                break;
            }
            case OpCodeEnum::LMUL: {
                popValue();
                const auto val2 = popValue();
                popValue();
                const auto val1 = popValue();
                pushValue(irBuilder.CreateMul(val1, val2));
                pushI8Padding();
                break;
            }
            case OpCodeEnum::FMUL: {
                const auto val2 = popValue();
                const auto val1 = popValue();
                pushValue(irBuilder.CreateFMul(val1, val2));
                break;
            }
            case OpCodeEnum::DMUL: {
                popValue();
                const auto val2 = popValue();
                popValue();
                const auto val1 = popValue();
                pushValue(irBuilder.CreateFMul(val1, val2));
                pushF8Padding();
                break;
            }

            case OpCodeEnum::IDIV:
            case OpCodeEnum::LDIV: {
                const auto val2 = popValue();
                const auto val1 = popValue();
                //TODO CheckZero
                pushValue(irBuilder.CreateSDiv(val1, val2));
                break;
            }
            case OpCodeEnum::FDIV:
            case OpCodeEnum::DDIV: {
                const auto val2 = popValue();
                const auto val1 = popValue();
                pushValue(irBuilder.CreateFDiv(val1, val2));
                break;
            }

            case OpCodeEnum::IREM:
            case OpCodeEnum::LREM: {
                const auto val2 = popValue();
                const auto val1 = popValue();
                //TODO CheckZero
                pushValue(irBuilder.CreateSRem(val1, val2));
                break;
            }

            case OpCodeEnum::FREM:
            case OpCodeEnum::DREM: {
                const auto val2 = popValue();
                const auto val1 = popValue();
                //TODO CheckZero
                pushValue(irBuilder.CreateFRem(val1, val2));
                break;
            }

            case OpCodeEnum::INEG:
            case OpCodeEnum::LNEG: {
                const auto val1 = popValue();
                pushValue(irBuilder.CreateNeg(val1));
                break;
            }
            case OpCodeEnum::FNEG:
            case OpCodeEnum::DNEG: {
                const auto val1 = popValue();
                pushValue(irBuilder.CreateFNeg(val1));
                break;
            }

            case OpCodeEnum::ISHL: {
                const auto val2 = popValue();
                const auto val1 = popValue();
                pushValue(irBuilder.CreateShl(val1, val2));
                break;
            }
            case OpCodeEnum::LSHL: {
                const auto val2 = popValue();
                const auto val1 = popValue();
                const auto val2Ext = irBuilder.CreateZExt(val2, irBuilder.getInt64Ty());
                pushValue(irBuilder.CreateShl(val1, val2Ext));
                break;
            }

            case OpCodeEnum::ISHR: {
                const auto val2 = popValue();
                const auto val1 = popValue();
                pushValue(irBuilder.CreateAShr(val1, val2));
                break;
            }
            case OpCodeEnum::LSHR: {
                const auto val2 = popValue();
                const auto val1 = popValue();
                const auto val2Ext = irBuilder.CreateZExt(val2, irBuilder.getInt64Ty());
                pushValue(irBuilder.CreateAShr(val1, val2Ext));
                break;
            }

            case OpCodeEnum::IUSHR: {
                const auto val2 = popValue();
                const auto val1 = popValue();
                pushValue(irBuilder.CreateLShr(val1, val2));
                break;
            }
            case OpCodeEnum::LUSHR: {
                const auto val2 = popValue();
                const auto val1 = popValue();
                const auto val2Ext = irBuilder.CreateZExt(val2, irBuilder.getInt64Ty());
                pushValue(irBuilder.CreateLShr(val1, val2Ext));
                break;
            }

            case OpCodeEnum::IAND:
            case OpCodeEnum::LAND: {
                const auto val2 = popValue();
                const auto val1 = popValue();
                pushValue(irBuilder.CreateAnd(val1, val2));
                break;
            }

            case OpCodeEnum::IOR:
            case OpCodeEnum::LOR: {
                const auto val2 = popValue();
                const auto val1 = popValue();
                pushValue(irBuilder.CreateOr(val1, val2));
                break;
            }

            case OpCodeEnum::IXOR:
            case OpCodeEnum::LXOR: {
                const auto val2 = popValue();
                const auto val1 = popValue();
                pushValue(irBuilder.CreateXor(val1, val2));
                break;
            }

            case OpCodeEnum::IINC: {
                const auto index = byteReader.readU1();
                const auto value = byteReader.readI1();
                const auto currentValue = getLocalVariableTableValue(index, SlotTypeEnum::I4);
                const auto incValue = irBuilder.CreateAdd(currentValue, irBuilder.getInt32(value));
                setLocalVariableTableValue(index, incValue, SlotTypeEnum::I4);
                break;
            }

            case OpCodeEnum::I2L: {
                //sext
                pushValue(irBuilder.CreateSExt(popValue(), slotTypeMap(SlotTypeEnum::I8)));
                pushI8Padding();
                break;
            }

            case OpCodeEnum::I2F: {
                //sitofp
                pushValue(irBuilder.CreateSIToFP(popValue(), slotTypeMap(SlotTypeEnum::F4)));
                break;
            }

            case OpCodeEnum::I2D: {
                //sitofp
                pushValue(irBuilder.CreateSIToFP(popValue(), slotTypeMap(SlotTypeEnum::F8)));
                pushF8Padding();
                break;
            }

            case OpCodeEnum::L2I: {
                //trunc
                popValue();
                pushValue(irBuilder.CreateTrunc(popValue(), slotTypeMap(SlotTypeEnum::I4)));
                break;
            }

            case OpCodeEnum::L2F: {
                //sitofp
                popValue();
                pushValue(irBuilder.CreateSIToFP(popValue(), slotTypeMap(SlotTypeEnum::F4)));
                break;
            }

            case OpCodeEnum::L2D: {
                //sitofp
                popValue();
                pushValue(irBuilder.CreateSIToFP(popValue(), slotTypeMap(SlotTypeEnum::F8)));
                pushF8Padding();
                break;
            }

            case OpCodeEnum::F2I: {
                //fptosi
                pushValue(irBuilder.CreateFPToSI(popValue(), slotTypeMap(SlotTypeEnum::I4)));
                break;
            }

            case OpCodeEnum::F2L: {
                //fptosi
                pushValue(irBuilder.CreateFPToSI(popValue(), slotTypeMap(SlotTypeEnum::I8)));
                pushI8Padding();
                break;
            }

            case OpCodeEnum::F2D: {
                //fpext
                pushValue(irBuilder.CreateFPExt(popValue(), slotTypeMap(SlotTypeEnum::F8)));
                pushF8Padding();
                break;
            }

            case OpCodeEnum::D2I: {
                //fptosi
                popValue();
                pushValue(irBuilder.CreateFPToSI(popValue(), slotTypeMap(SlotTypeEnum::I4)));
                break;
            }

            case OpCodeEnum::D2L: {
                //fptosi
                popValue();
                pushValue(irBuilder.CreateFPToSI(popValue(), slotTypeMap(SlotTypeEnum::I8)));
                pushI8Padding();
                break;
            }

            case OpCodeEnum::D2F: {
                //fptrunc
                popValue();
                pushValue(irBuilder.CreateFPTrunc(popValue(), slotTypeMap(SlotTypeEnum::F4)));
                break;
            }

            case OpCodeEnum::I2B: {
                //trunc, sext
                pushValue(irBuilder.CreateSExt(irBuilder.CreateTrunc(popValue(), irBuilder.getInt8Ty()),
                                               slotTypeMap(SlotTypeEnum::I4)));
                break;
            }

            case OpCodeEnum::I2C: {
                //trunc, zext
                pushValue(irBuilder.CreateZExt(irBuilder.CreateTrunc(popValue(), irBuilder.getInt16Ty()),
                                               slotTypeMap(SlotTypeEnum::I4)));
                break;
            }

            case OpCodeEnum::I2S: {
                //trunc, sext
                pushValue(irBuilder.CreateSExt(irBuilder.CreateTrunc(popValue(), irBuilder.getInt16Ty()),
                                               slotTypeMap(SlotTypeEnum::I4)));
                break;
            }

            case OpCodeEnum::LCMP: {
                popValue();
                const auto val2 = popValue();
                popValue();
                const auto val1 = popValue();
                lCmp(val1, val2);
                break;
            }

            case OpCodeEnum::FCMPL: {
                const auto val2 = popValue();
                const auto val1 = popValue();
                const auto nanRet = irBuilder.getInt32(-1);
                fCmp(val1, val2, nanRet);
                break;
            }

            case OpCodeEnum::FCMPG: {
                const auto val2 = popValue();
                const auto val1 = popValue();
                const auto nanRet = irBuilder.getInt32(1);
                fCmp(val1, val2, nanRet);
                break;
            }

            case OpCodeEnum::DCMPL: {
                popValue();
                const auto val2 = popValue();
                popValue();
                const auto val1 = popValue();
                const auto nanRet = irBuilder.getInt32(-1);
                fCmp(val1, val2, nanRet);
                break;
            }

            case OpCodeEnum::DCMPG: {
                popValue();
                const auto val2 = popValue();
                popValue();
                const auto val1 = popValue();
                const auto nanRet = irBuilder.getInt32(1);
                fCmp(val1, val2, nanRet);
                break;
            }

            case OpCodeEnum::IFEQ:
            case OpCodeEnum::IFNE:
            case OpCodeEnum::IFLT:
            case OpCodeEnum::IFGE:
            case OpCodeEnum::IFGT:
            case OpCodeEnum::IFLE: {
                const auto offset = byteReader.readI2();
                const auto jumpTo = CAST_U4(CAST_I4(pc) + offset);
                const auto val = popValue();
                ifOp(jumpTo, val, irBuilder.getInt32(0), opCode);
                break;
            }

            case OpCodeEnum::IF_ICMPEQ:
            case OpCodeEnum::IF_ICMPNE:
            case OpCodeEnum::IF_ICMPLT:
            case OpCodeEnum::IF_ICMPGE:
            case OpCodeEnum::IF_ICMPGT:
            case OpCodeEnum::IF_ICMPLE: {
                const auto curOp = CAST_U1(opCode) - (static_cast<u1>(OpCodeEnum::IF_ICMPEQ) - static_cast<u1>(OpCodeEnum::IFEQ));
                const auto offset = byteReader.readI2();
                const auto jumpTo = CAST_U4(CAST_I4(pc) + offset);
                const auto val2 = popValue();
                const auto val1 = popValue();
                ifOp(jumpTo, val1, val2, static_cast<OpCodeEnum>(curOp));
                break;
            }
            case OpCodeEnum::IF_ACMPEQ:
            case OpCodeEnum::IF_ACMPNE: {
                const auto curOp = CAST_U1(opCode) - (static_cast<u1>(OpCodeEnum::IF_ACMPEQ) - static_cast<u1>(OpCodeEnum::IFEQ));
                const auto offset = byteReader.readI2();
                const auto jumpTo = CAST_U4(CAST_I4(pc) + offset);
                const auto val2 = popValue();
                const auto val1 = popValue();
                ifOp(jumpTo, val1, val2, static_cast<OpCodeEnum>(curOp));
                break;
            }

            case OpCodeEnum::GOTO: {
                const auto offset = byteReader.readI2();
                const auto jumpTo = CAST_U4(CAST_I4(pc) + offset);
                irBuilder.CreateBr(opCodeBlocks[CAST_U4(jumpTo)]);
                break;
            }

            case OpCodeEnum::TABLESWITCH:
            case OpCodeEnum::LOOKUPSWITCH: {
                panic("not support");
                break;
            }

            case OpCodeEnum::IRETURN: {
                const auto retVal = popValue();
                irBuilder.CreateCall(returnI4, {getFramePtr(), retVal});
                break;
            }
            case OpCodeEnum::LRETURN: {
                popValue();
                const auto retVal = popValue();
                irBuilder.CreateCall(returnI8, {getFramePtr(), retVal});
                break;
            }
            case OpCodeEnum::FRETURN: {
                const auto retVal = popValue();
                irBuilder.CreateCall(returnF4, {getFramePtr(), retVal});
                break;
            }
            case OpCodeEnum::DRETURN: {
                popValue();
                const auto retVal = popValue();
                irBuilder.CreateCall(returnF8, {getFramePtr(), retVal});
                break;
            }
            case OpCodeEnum::ARETURN: {
                const auto retVal = popValue();
                irBuilder.CreateCall(returnObj, {getFramePtr(), retVal});
                break;
            }
            case OpCodeEnum::RETURN: {
                irBuilder.CreateCall(returnVoid, {getFramePtr()});
                break;
            }

            default:
                break;
        }
    }

    void MethodCompiler::compile() {
        ByteReader reader{};
        const auto codePtr = method.code.get();
        reader.init(codePtr, method.codeLength);

        while (!reader.eof()) {
            const auto currentByteCode = reader.readU1();
            const auto opCode = static_cast<OpCodeEnum>(currentByteCode);
            const auto pc = CAST_U4(reader.ptr - reader.begin) - 1;
            processInstruction(opCode, pc, reader);
            reader.resetCurrentOffset();
        }

        if (!valueStack.empty()) {
            panic("error value stack");
        }

        irBuilder.CreateBr(returnBB);

        changeBB(returnBB);
        irBuilder.CreateRetVoid();

        verifyFunction(*function);
    }
}
