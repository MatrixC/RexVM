#include "llvm_block_context.hpp"
#include "../class_member.hpp"
#include "../utils/byte_reader.hpp"
#include "llvm_compiler.hpp"

namespace RexVM {
    BlockContext::BlockContext(MethodCompiler &methodCompiler, MethodBlock *methodBlock)
        : methodCompiler(methodCompiler),
          irBuilder(methodCompiler.irBuilder),
          methodBlock(methodBlock) {
        const auto &method = methodCompiler.method;
        auto &ctx = methodCompiler.ctx;
        const auto lineNumber = method.getLineNumber(methodBlock->startPC);
        const auto blockName = cformat("{}_{}", methodBlock->startPC, lineNumber);
        basicBlock = llvm::BasicBlock::Create(ctx, blockName);
    }

    void BlockContext::pushValue(llvm::Value *value) {
        blockValueStack.push(value);
    }

    void BlockContext::pushValue(llvm::Value *value, const SlotTypeEnum type) {
        switch (type) {
            case SlotTypeEnum::I4:
            case SlotTypeEnum::F4:
            case SlotTypeEnum::REF:
                pushValue(value);
            break;

            case SlotTypeEnum::I8:
            case SlotTypeEnum::F8:
                pushWideValue(value);
            break;

            default:
                panic("error type");
        }
    }

    llvm::Value *BlockContext::popValue() {
        if (!blockValueStack.empty()) {
            const auto value = blockValueStack.top();
            blockValueStack.pop();
            return value;
        }

        if (parentBlocks.empty()) {
            panic("error stack");
            return nullptr;
        }

        if (parentBlocks.size() == 1) {
            return parentBlocks[0]->popValue();
        }

        std::vector<std::tuple<llvm::Value *, llvm::BasicBlock *>> popValues;
        popValues.reserve(parentBlocks.size());
        for (const auto &parentBlock : parentBlocks) {
            const auto val = parentBlock->popValue();
            popValues.emplace_back(val, parentBlock->basicBlock);
        }

        const auto selectType = std::get<0>(popValues[0])->getType();
        const auto selectPopValue = irBuilder.CreatePHI(selectType, popValues.size());
        for (const auto &[val, bb] : popValues) {
            selectPopValue->addIncoming(val, bb);
        }

        return selectPopValue;
    }

    llvm::Value *BlockContext::popValue(const SlotTypeEnum type) {
        llvm::Value *value{nullptr};
        switch (type) {
            case SlotTypeEnum::I4:
            case SlotTypeEnum::F4:
            case SlotTypeEnum::REF:
                value = popValue();
            break;

            case SlotTypeEnum::I8:
            case SlotTypeEnum::F8:
                value = popWideValue();
            break;

            default:
                panic("error type");
        }
        return value;
    }

    void BlockContext::pushWideValue(llvm::Value *value) {
        pushValue(value);
        padding();
    }

    llvm::Value *BlockContext::popWideValue() {
        unPadding();
        return popValue(); 
    }

    llvm::Value *BlockContext::topValue() {
        if (!blockValueStack.empty()) {
            return blockValueStack.top();
        }

        if (parentBlocks.empty()) {
            panic("error stack");
            return nullptr;
        }

        if (parentBlocks.size() == 1) {
            return parentBlocks[0]->topValue();
        }

        std::vector<std::tuple<llvm::Value *, llvm::BasicBlock *>> popValues;
        popValues.reserve(parentBlocks.size());
        for (const auto &parentBlock : parentBlocks) {
            const auto val = parentBlock->topValue();
            popValues.emplace_back(val, parentBlock->basicBlock);
        }

        const auto selectType = std::get<0>(popValues[0])->getType();
        const auto selectPopValue = irBuilder.CreatePHI(selectType, popValues.size());
        for (const auto &[val, bb] : popValues) {
            selectPopValue->addIncoming(val, bb);
        }

        return selectPopValue;
    }

    void BlockContext::padding() {
        pushValue(nullptr); 
    }

    void BlockContext::unPadding() {
        popValue(); 
    }

    void BlockContext::pushI4Const(const i4 value) {
        pushValue(irBuilder.getInt32(static_cast<uint32_t>(value))); 
    }

    void BlockContext::pushI8Const(const i8 value) {
        pushWideValue(irBuilder.getInt64(static_cast<uint64_t>(value))); 
    }

    void BlockContext::pushF4Const(const f4 value) {
        pushValue(llvm::ConstantFP::get(irBuilder.getFloatTy(), value)); 
    }

    void BlockContext::pushF8Const(const f8 value) {
        pushWideValue(llvm::ConstantFP::get(irBuilder.getDoubleTy(), value)); 
    }

    void BlockContext::pushNullConst() {
        pushValue(methodCompiler.getZeroValue(SlotTypeEnum::REF)); 
    }

    void BlockContext::compile() {
        const auto &method = methodCompiler.method;
        const auto codeBegin = method.code.get();
        methodCompiler.changeBB(basicBlock);

        ByteReader reader{};
        const auto codePtr = method.code.get() + methodBlock->startPC;
        const auto codeLength = methodBlock->endPC - methodBlock->startPC;
        reader.init(codePtr, codeLength);

        while (!reader.eof()) {
            pc = CAST_U4(reader.ptr - codeBegin);
            const auto currentByteCode = reader.readU1();
            const auto opCode = static_cast<OpCodeEnum>(currentByteCode);

            processInstruction(opCode, reader);
            reader.resetCurrentOffset();
        }

        if (methodBlock->autoJmp) {
            const auto index = methodBlock->jumpToBlockIndex[0];
            const auto jumpToPC = methodCompiler.cfgBlocks[index]->methodBlock->startPC;
            methodCompiler.jumpToPC(jumpToPC);
        }
    }

    void BlockContext::processInstruction(OpCodeEnum opCode, ByteReader &byteReader) {
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
                methodCompiler.ldc(*this, index);
                break;
            }

            case OpCodeEnum::LDC_W:
            case OpCodeEnum::LDC2_W: {
                const auto index = byteReader.readU2();
                methodCompiler.ldc(*this, index);
                break;
            }

            case OpCodeEnum::ILOAD: {
                const auto index = byteReader.readU1();
                methodCompiler.load(*this, index, SlotTypeEnum::I4);
                break;
            }

            case OpCodeEnum::LLOAD: {
                const auto index = byteReader.readU1();
                methodCompiler.load(*this, index, SlotTypeEnum::I8);
                break;
            }

            case OpCodeEnum::FLOAD: {
                const auto index = byteReader.readU1();
                methodCompiler.load(*this, index, SlotTypeEnum::F4);
                break;
            }

            case OpCodeEnum::DLOAD: {
                const auto index = byteReader.readU1();
                methodCompiler.load(*this, index, SlotTypeEnum::F8);
                break;
            }

            case OpCodeEnum::ALOAD: {
                const auto index = byteReader.readU1();
                methodCompiler.load(*this, index, SlotTypeEnum::REF);
                break;
            }

            case OpCodeEnum::ILOAD_0:
            case OpCodeEnum::ILOAD_1:
            case OpCodeEnum::ILOAD_2:
            case OpCodeEnum::ILOAD_3: {
                const auto idx = CAST_I4(opCode) - CAST_I4(OpCodeEnum::ILOAD_0);
                methodCompiler.load(*this, idx, SlotTypeEnum::I4);
                break;
            }

            case OpCodeEnum::LLOAD_0:
            case OpCodeEnum::LLOAD_1:
            case OpCodeEnum::LLOAD_2:
            case OpCodeEnum::LLOAD_3: {
                const auto idx = CAST_I4(opCode) - CAST_I4(OpCodeEnum::LLOAD_0);
                methodCompiler.load(*this, idx, SlotTypeEnum::I8);
                break;
            }

            case OpCodeEnum::FLOAD_0:
            case OpCodeEnum::FLOAD_1:
            case OpCodeEnum::FLOAD_2:
            case OpCodeEnum::FLOAD_3: {
                const auto idx = CAST_I4(opCode) - CAST_I4(OpCodeEnum::FLOAD_0);
                methodCompiler.load(*this, idx, SlotTypeEnum::F4);
                break;
            }

            case OpCodeEnum::DLOAD_0:
            case OpCodeEnum::DLOAD_1:
            case OpCodeEnum::DLOAD_2:
            case OpCodeEnum::DLOAD_3: {
                const auto idx = CAST_I4(opCode) - CAST_I4(OpCodeEnum::DLOAD_0);
                methodCompiler.load(*this, idx, SlotTypeEnum::F8);
                break;
            }

            case OpCodeEnum::ALOAD_0:
            case OpCodeEnum::ALOAD_1:
            case OpCodeEnum::ALOAD_2:
            case OpCodeEnum::ALOAD_3: {
                const auto idx = CAST_I4(opCode) - CAST_I4(OpCodeEnum::ALOAD_0);
                methodCompiler.load(*this, idx, SlotTypeEnum::REF);
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
                const auto arrayType = CAST_U1(opCode) - CAST_U1(OpCodeEnum::IALOAD);
                const auto idx = popValue();
                const auto arrayRef = popValue();
                methodCompiler.arrayLoad(*this, arrayRef, idx, arrayType);
                break;
            }

            case OpCodeEnum::ISTORE: {
                const auto index = byteReader.readU1();
                methodCompiler.store(*this, index, SlotTypeEnum::I4);
                break;
            }
            case OpCodeEnum::LSTORE: {
                const auto index = byteReader.readU1();
                methodCompiler.store(*this, index, SlotTypeEnum::I8);
                break;
            }
            case OpCodeEnum::FSTORE: {
                const auto index = byteReader.readU1();
                methodCompiler.store(*this, index, SlotTypeEnum::F4);
                break;
            }
            case OpCodeEnum::DSTORE: {
                const auto index = byteReader.readU1();
                methodCompiler.store(*this, index, SlotTypeEnum::F8);
                break;
            }
            case OpCodeEnum::ASTORE: {
                const auto index = byteReader.readU1();
                methodCompiler.store(*this, index, SlotTypeEnum::REF);
                break;
            }

            case OpCodeEnum::ISTORE_0:
            case OpCodeEnum::ISTORE_1:
            case OpCodeEnum::ISTORE_2:
            case OpCodeEnum::ISTORE_3: {
                const auto idx = CAST_I4(opCode) - CAST_I4(OpCodeEnum::ISTORE_0);
                methodCompiler.store(*this, idx, SlotTypeEnum::I4);
                break;
            }

            case OpCodeEnum::LSTORE_0:
            case OpCodeEnum::LSTORE_1:
            case OpCodeEnum::LSTORE_2:
            case OpCodeEnum::LSTORE_3: {
                const auto idx = CAST_I4(opCode) - CAST_I4(OpCodeEnum::LSTORE_0);
                methodCompiler.store(*this, idx, SlotTypeEnum::I8);
                break;
            }

            case OpCodeEnum::FSTORE_0:
            case OpCodeEnum::FSTORE_1:
            case OpCodeEnum::FSTORE_2:
            case OpCodeEnum::FSTORE_3: {
                const auto idx = CAST_I4(opCode) - CAST_I4(OpCodeEnum::FSTORE_0);
                methodCompiler.store(*this, idx, SlotTypeEnum::F4);
                break;
            }

            case OpCodeEnum::DSTORE_0:
            case OpCodeEnum::DSTORE_1:
            case OpCodeEnum::DSTORE_2:
            case OpCodeEnum::DSTORE_3: {
                const auto idx = CAST_I4(opCode) - CAST_I4(OpCodeEnum::DSTORE_0);
                methodCompiler.store(*this, idx, SlotTypeEnum::F8);
                break;
            }

            case OpCodeEnum::ASTORE_0:
            case OpCodeEnum::ASTORE_1:
            case OpCodeEnum::ASTORE_2:
            case OpCodeEnum::ASTORE_3: {
                const auto idx = CAST_I4(opCode) - CAST_I4(OpCodeEnum::ASTORE_0);
                methodCompiler.store(*this, idx, SlotTypeEnum::REF);
                break;
            }

            case OpCodeEnum::IASTORE: {
                const auto arrayType = CAST_U1(opCode) - CAST_U1(OpCodeEnum::IASTORE);
                const auto val = popValue();
                const auto idx = popValue();
                const auto arrayRef = popValue();
                methodCompiler.arrayStore(*this, arrayRef, idx, val, arrayType);
                break;
            }
            case OpCodeEnum::LASTORE: {
                const auto arrayType = CAST_U1(opCode) - CAST_U1(OpCodeEnum::IASTORE);
                const auto val = popWideValue();
                const auto idx = popValue();
                const auto arrayRef = popValue();
                methodCompiler.arrayStore(*this, arrayRef, idx, val, arrayType);
                break;
            }
            case OpCodeEnum::FASTORE: {
                const auto arrayType = CAST_U1(opCode) - CAST_U1(OpCodeEnum::IASTORE);
                const auto val = popValue();
                const auto idx = popValue();
                const auto arrayRef = popValue();
                methodCompiler.arrayStore(*this, arrayRef, idx, val, arrayType);
                break;
            }
            case OpCodeEnum::DASTORE: {
                const auto arrayType = CAST_U1(opCode) - CAST_U1(OpCodeEnum::IASTORE);
                const auto val = popWideValue();
                const auto idx = popValue();
                const auto arrayRef = popValue();
                methodCompiler.arrayStore(*this, arrayRef, idx, val, arrayType);
                break;
            }
            case OpCodeEnum::AASTORE:
            case OpCodeEnum::BASTORE:
            case OpCodeEnum::CASTORE:
            case OpCodeEnum::SASTORE: {
                const auto arrayType = CAST_U1(opCode) - CAST_U1(OpCodeEnum::IASTORE);
                const auto val = popValue();
                const auto idx = popValue();
                const auto arrayRef = popValue();
                methodCompiler.arrayStore(*this, arrayRef, idx, val, arrayType);
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
                const auto val2 = popWideValue();
                const auto val1 = popWideValue();
                pushWideValue(irBuilder.CreateAdd(val1, val2));
                break;
            }

            case OpCodeEnum::FADD:  {
                const auto val2 = popValue();
                const auto val1 = popValue();
                pushValue(irBuilder.CreateFAdd(val1, val2));
                break;
            }

            case OpCodeEnum::DADD: {
                const auto val2 = popWideValue();
                const auto val1 = popWideValue();
                pushWideValue(irBuilder.CreateFAdd(val1, val2));
                break;
            }

            case OpCodeEnum::ISUB: {
                const auto val2 = popValue();
                const auto val1 = popValue();
                pushValue(irBuilder.CreateSub(val1, val2));
                break;
            }

            case OpCodeEnum::LSUB: {
                const auto val2 = popWideValue();
                const auto val1 = popWideValue();
                pushWideValue(irBuilder.CreateSub(val1, val2));
                break;
            }

            case OpCodeEnum::FSUB: {
                const auto val2 = popValue();
                const auto val1 = popValue();
                pushValue(irBuilder.CreateFSub(val1, val2));
                break;
            }

            case OpCodeEnum::DSUB: {
                const auto val2 = popWideValue();
                const auto val1 = popWideValue();
                pushWideValue(irBuilder.CreateFSub(val1, val2));
                break;
            }

            case OpCodeEnum::IMUL: {
                const auto val2 = popValue();
                const auto val1 = popValue();
                pushValue(irBuilder.CreateMul(val1, val2));
                break;
            }

            case OpCodeEnum::LMUL: {
                const auto val2 = popWideValue();
                const auto val1 = popWideValue();
                pushWideValue(irBuilder.CreateMul(val1, val2));
                break;
            }

            case OpCodeEnum::FMUL: {
                const auto val2 = popValue();
                const auto val1 = popValue();
                pushValue(irBuilder.CreateFMul(val1, val2));
                break;
            }

            case OpCodeEnum::DMUL: {
                const auto val2 = popWideValue();
                const auto val1 = popWideValue();
                pushWideValue(irBuilder.CreateFMul(val1, val2));
                break;
            }

            case OpCodeEnum::IDIV: {
                const auto val2 = popValue();
                const auto val1 = popValue();
                methodCompiler.throwNpeIfZero(*this, val2, SlotTypeEnum::I4);
                pushValue(irBuilder.CreateSDiv(val1, val2));
                break;
            }

            case OpCodeEnum::LDIV: {
                const auto val2 = popWideValue();
                const auto val1 = popWideValue();
                methodCompiler.throwNpeIfZero(*this, val2, SlotTypeEnum::I8);
                pushWideValue(irBuilder.CreateSDiv(val1, val2));
                break;
            }

            case OpCodeEnum::FDIV: {
                const auto val2 = popValue();
                const auto val1 = popValue();
                pushValue(irBuilder.CreateFDiv(val1, val2));
                break;
            }

            case OpCodeEnum::DDIV: {
                const auto val2 = popWideValue();
                const auto val1 = popWideValue();
                pushWideValue(irBuilder.CreateFDiv(val1, val2));
                break;
            }

            case OpCodeEnum::IREM: {
                const auto val2 = popValue();
                const auto val1 = popValue();
                methodCompiler.throwNpeIfZero(*this, val2, SlotTypeEnum::I4);
                pushValue(irBuilder.CreateSRem(val1, val2));
                break;
            }

            case OpCodeEnum::LREM: {
                const auto val2 = popWideValue();
                const auto val1 = popWideValue();
                methodCompiler.throwNpeIfZero(*this, val2, SlotTypeEnum::I8);
                pushWideValue(irBuilder.CreateSRem(val1, val2));
                break;
            }

            case OpCodeEnum::FREM: {
                const auto val2 = popValue();
                const auto val1 = popValue();
                methodCompiler.throwNpeIfZero(*this, val2, SlotTypeEnum::F4);
                pushValue(irBuilder.CreateFRem(val1, val2));
                break;
            }

            case OpCodeEnum::DREM: {
                const auto val2 = popWideValue();
                const auto val1 = popWideValue();
                methodCompiler.throwNpeIfZero(*this, val2, SlotTypeEnum::F8);
                pushWideValue(irBuilder.CreateFRem(val1, val2));
                break;
            }

            case OpCodeEnum::INEG: {
                const auto val1 = popValue();
                pushValue(irBuilder.CreateNeg(val1));
                break;
            }

            case OpCodeEnum::LNEG: {
                const auto val1 = popWideValue();
                pushWideValue(irBuilder.CreateNeg(val1));
                break;
            }
            case OpCodeEnum::FNEG: {
                const auto val1 = popValue();
                pushValue(irBuilder.CreateFNeg(val1));
                break;
            }

            case OpCodeEnum::DNEG: {
                const auto val1 = popWideValue();
                pushWideValue(irBuilder.CreateFNeg(val1));
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
                const auto val1 = popWideValue();
                const auto val2Ext = irBuilder.CreateZExt(val2, irBuilder.getInt64Ty());
                pushWideValue(irBuilder.CreateShl(val1, val2Ext));
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
                const auto val1 = popWideValue();
                const auto val2Ext = irBuilder.CreateZExt(val2, irBuilder.getInt64Ty());
                pushWideValue(irBuilder.CreateAShr(val1, val2Ext));
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
                const auto val1 = popWideValue();
                const auto val2Ext = irBuilder.CreateZExt(val2, irBuilder.getInt64Ty());
                pushWideValue(irBuilder.CreateLShr(val1, val2Ext));
                break;
            }

            case OpCodeEnum::IAND: {
                const auto val2 = popValue();
                const auto val1 = popValue();
                pushValue(irBuilder.CreateAnd(val1, val2));
                break;
            }

            case OpCodeEnum::LAND: {
                const auto val2 = popWideValue();
                const auto val1 = popWideValue();
                pushWideValue(irBuilder.CreateAnd(val1, val2));
                break;
            }

            case OpCodeEnum::IOR: {
                const auto val2 = popValue();
                const auto val1 = popValue();
                pushValue(irBuilder.CreateOr(val1, val2));
                break;
            }

            case OpCodeEnum::LOR: {
                const auto val2 = popWideValue();
                const auto val1 = popWideValue();
                pushWideValue(irBuilder.CreateOr(val1, val2));
                break;
            }

            case OpCodeEnum::IXOR: {
                const auto val2 = popValue();
                const auto val1 = popValue();
                pushValue(irBuilder.CreateXor(val1, val2));
                break;
            }

            case OpCodeEnum::LXOR: {
                const auto val2 = popWideValue();
                const auto val1 = popWideValue();
                pushWideValue(irBuilder.CreateXor(val1, val2));
                break;
            }

            case OpCodeEnum::IINC: {
                const auto index = byteReader.readU1();
                const auto value = byteReader.readI1();
                const auto currentValue = methodCompiler.getLocalVariableTableValue(index, SlotTypeEnum::I4);
                const auto incValue = irBuilder.CreateAdd(currentValue, irBuilder.getInt32(value));
                methodCompiler.setLocalVariableTableValue(index, incValue, SlotTypeEnum::I4);
                break;
            }

            case OpCodeEnum::I2L: {
                //sext
                pushWideValue(irBuilder.CreateSExt(popValue(), methodCompiler.slotTypeMap(SlotTypeEnum::I8)));
                break;
            }

            case OpCodeEnum::I2F: {
                //sitofp
                pushValue(irBuilder.CreateSIToFP(popValue(), methodCompiler.slotTypeMap(SlotTypeEnum::F4)));
                break;
            }

            case OpCodeEnum::I2D: {
                //sitofp
                pushWideValue(irBuilder.CreateSIToFP(popValue(), methodCompiler.slotTypeMap(SlotTypeEnum::F8)));
                break;
            }

            case OpCodeEnum::L2I: {
                //trunc
                pushValue(irBuilder.CreateTrunc(popWideValue(), methodCompiler.slotTypeMap(SlotTypeEnum::I4)));
                break;
            }

            case OpCodeEnum::L2F: {
                //sitofp
                pushValue(irBuilder.CreateSIToFP(popWideValue(), methodCompiler.slotTypeMap(SlotTypeEnum::F4)));
                break;
            }

            case OpCodeEnum::L2D: {
                //sitofp
                pushWideValue(irBuilder.CreateSIToFP(popWideValue(), methodCompiler.slotTypeMap(SlotTypeEnum::F8)));
                break;
            }

            case OpCodeEnum::F2I: {
                //fptosi
                pushValue(irBuilder.CreateFPToSI(popValue(), methodCompiler.slotTypeMap(SlotTypeEnum::I4)));
                break;
            }

            case OpCodeEnum::F2L: {
                //fptosi
                pushWideValue(irBuilder.CreateFPToSI(popValue(), methodCompiler.slotTypeMap(SlotTypeEnum::I8)));
                break;
            }

            case OpCodeEnum::F2D: {
                //fpext
                pushWideValue(irBuilder.CreateFPExt(popValue(), methodCompiler.slotTypeMap(SlotTypeEnum::F8)));
                break;
            }

            case OpCodeEnum::D2I: {
                //fptosi
                pushValue(irBuilder.CreateFPToSI(popWideValue(), methodCompiler.slotTypeMap(SlotTypeEnum::I4)));
                break;
            }

            case OpCodeEnum::D2L: {
                //fptosi
                pushWideValue(irBuilder.CreateFPToSI(popWideValue(), methodCompiler.slotTypeMap(SlotTypeEnum::I8)));
                break;
            }

            case OpCodeEnum::D2F: {
                //fptrunc
                pushValue(irBuilder.CreateFPTrunc(popWideValue(), methodCompiler.slotTypeMap(SlotTypeEnum::F4)));
                break;
            }

            case OpCodeEnum::I2B: {
                //trunc, sext
                pushValue(irBuilder.CreateSExt(irBuilder.CreateTrunc(popValue(), irBuilder.getInt8Ty()),
                                               methodCompiler.slotTypeMap(SlotTypeEnum::I4)));
                break;
            }

            case OpCodeEnum::I2C: {
                //trunc, zext
                pushValue(irBuilder.CreateZExt(irBuilder.CreateTrunc(popValue(), irBuilder.getInt16Ty()),
                                               methodCompiler.slotTypeMap(SlotTypeEnum::I4)));
                break;
            }

            case OpCodeEnum::I2S: {
                //trunc, sext
                pushValue(irBuilder.CreateSExt(irBuilder.CreateTrunc(popValue(), irBuilder.getInt16Ty()),
                                               methodCompiler.slotTypeMap(SlotTypeEnum::I4)));
                break;
            }

            case OpCodeEnum::LCMP: {
                const auto val2 = popWideValue();
                const auto val1 = popWideValue();
                methodCompiler.lCmp(*this, val1, val2);
                break;
            }

            case OpCodeEnum::FCMPL: {
                const auto val2 = popValue();
                const auto val1 = popValue();
                const auto nanRet = irBuilder.getInt32(-1);
                methodCompiler.fCmp(*this, val1, val2, nanRet);
                break;
            }

            case OpCodeEnum::FCMPG: {
                const auto val2 = popValue();
                const auto val1 = popValue();
                const auto nanRet = irBuilder.getInt32(1);
                methodCompiler.fCmp(*this, val1, val2, nanRet);
                break;
            }

            case OpCodeEnum::DCMPL: {
                const auto val2 = popWideValue();
                const auto val1 = popWideValue();
                const auto nanRet = irBuilder.getInt32(-1);
                methodCompiler.fCmp(*this, val1, val2, nanRet);
                break;
            }

            case OpCodeEnum::DCMPG: {
                const auto val2 = popWideValue();
                const auto val1 = popWideValue();
                const auto nanRet = irBuilder.getInt32(1);
                methodCompiler.fCmp(*this, val1, val2, nanRet);
                break;
            }

            case OpCodeEnum::IFEQ:
            case OpCodeEnum::IFNE:
            case OpCodeEnum::IFLT:
            case OpCodeEnum::IFGE:
            case OpCodeEnum::IFGT:
            case OpCodeEnum::IFLE: {
                const auto offset = byteReader.readI2();
                const auto val = popValue();
                methodCompiler.ifOp(*this, offset, val, methodCompiler.getZeroValue(SlotTypeEnum::I4), opCode);
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
                const auto val2 = popValue();
                const auto val1 = popValue();
                methodCompiler.ifOp(*this, offset, val1, val2, static_cast<OpCodeEnum>(curOp));
                break;
            }
            case OpCodeEnum::IF_ACMPEQ:
            case OpCodeEnum::IF_ACMPNE: {
                const auto curOp = CAST_U1(opCode) - (static_cast<u1>(OpCodeEnum::IF_ACMPEQ) - static_cast<u1>(OpCodeEnum::IFEQ));
                const auto offset = byteReader.readI2();
                const auto val2 = popValue();
                const auto val1 = popValue();
                methodCompiler.ifOp(*this, offset, val1, val2, static_cast<OpCodeEnum>(curOp));
                break;
            }

            case OpCodeEnum::GOTO: {
                const auto offset = byteReader.readI2();
                methodCompiler.jumpTo(*this, offset);
                break;
            }

            case OpCodeEnum::TABLESWITCH: {
                const auto nextPc = CAST_U4(byteReader.ptr - byteReader.begin);
                if (const auto mod = nextPc % 4; mod != 0) {
                    byteReader.skip(4 - mod);
                }
                std::vector<i4> cases;
                const auto defaultOffset = byteReader.readI4();
                cases.emplace_back(defaultOffset);

                const auto low = byteReader.readI4();
                const auto high = byteReader.readI4();
                const auto jumpOffsetsCount = high - low + 1;
                auto currentValue = low;

                const auto switchValue = popValue();
                for (i4 i = 0; i < jumpOffsetsCount; ++i) {
                    const auto currentOffset = byteReader.readI4();
                    cases.emplace_back(currentValue);
                    cases.emplace_back(currentOffset);
                    currentValue += 1;
                }
                methodCompiler.createSwitch(*this, switchValue, cases);
                break;
            }

            case OpCodeEnum::LOOKUPSWITCH: {
                const auto nextPc = CAST_U4(byteReader.ptr - byteReader.begin);
                if (const auto mod = nextPc % 4; mod != 0) {
                    byteReader.skip(4 - mod);
                }

                std::vector<i4> cases;
                const auto defaultOffset = byteReader.readI4();
                cases.emplace_back(defaultOffset);

                const auto nPairs = byteReader.readI4();
                const auto switchValue = popValue();

                for (i4 i = 0; i < nPairs; ++i) {
                    const auto caseValue = byteReader.readI4();
                    const auto caseOffset = byteReader.readI4();
                    cases.emplace_back(caseValue);
                    cases.emplace_back(caseOffset);
                }
                methodCompiler.createSwitch(*this, switchValue, cases);
                break;
            }

            case OpCodeEnum::IRETURN: {
                const auto retVal = popValue();
                methodCompiler.returnValue(retVal, SlotTypeEnum::I4);
                break;
            }

            case OpCodeEnum::LRETURN: {
                const auto retVal = popWideValue();
                methodCompiler.returnValue(retVal, SlotTypeEnum::I8);
                break;
            }

            case OpCodeEnum::FRETURN: {
                const auto retVal = popValue();
                methodCompiler.returnValue(retVal, SlotTypeEnum::F4);
                break;
            }

            case OpCodeEnum::DRETURN: {
                const auto retVal = popWideValue();
                methodCompiler.returnValue(retVal, SlotTypeEnum::F8);
                break;
            }

            case OpCodeEnum::ARETURN: {
                const auto retVal = popValue();
                methodCompiler.returnValue(retVal, SlotTypeEnum::REF);
                break;
            }

            case OpCodeEnum::RETURN: {
                methodCompiler.returnValue(nullptr, SlotTypeEnum::NONE);
                break;
            }

            case OpCodeEnum::GETSTATIC: {
                const auto index = byteReader.readU2();
                methodCompiler.getStatic(*this, index);
                break;
            }

            case OpCodeEnum::PUTSTATIC: {
                const auto index = byteReader.readU2();
                methodCompiler.putStatic(*this, index);
                break;
            }

            case OpCodeEnum::GETFIELD: {
                const auto index = byteReader.readU2();
                methodCompiler.getField(*this, index);
                break;
            }

            case OpCodeEnum::PUTFIELD: {
                const auto index = byteReader.readU2();
                methodCompiler.putField(*this, index);
                break;
            }

            case OpCodeEnum::INVOKEVIRTUAL: {
                const auto index = byteReader.readU2();
                methodCompiler.invokeVirtualMethod(*this, index);
                break;
            }

            case OpCodeEnum::INVOKESPECIAL: {
                const auto index = byteReader.readU2();
                methodCompiler.invokeStaticMethod(*this, index, false);
                break;
            }

            case OpCodeEnum::INVOKESTATIC: {
                const auto index = byteReader.readU2();
                methodCompiler.invokeStaticMethod(*this, index, true);
                break;
            }

            case OpCodeEnum::INVOKEINTERFACE: {
                const auto index = byteReader.readU2();
                byteReader.readU2(); //ignore
                methodCompiler.invokeVirtualMethod(*this, index);
                break;
            }

            case OpCodeEnum::INVOKEDYNAMIC: {
                const auto index = byteReader.readU2();
                byteReader.readU2(); //ignore
                methodCompiler.invokeDynamic(*this, index);
                break;
            }

            case OpCodeEnum::NEW: {
                const auto index = byteReader.readU2();
                methodCompiler.newObject(*this, index);
                break;
            }

            case OpCodeEnum::NEWARRAY: {
                const auto type = byteReader.readU1();
                const auto length = popValue();
                methodCompiler.newArray(*this, type, length);
                break;
            }

            case OpCodeEnum::ANEWARRAY: {
                const auto classIndex = byteReader.readU2();
                const auto length = popValue();
                methodCompiler.newObjectArray(*this, classIndex, length);
                break;
            }

            case OpCodeEnum::ARRAYLENGTH: {
                const auto array = popValue();
                methodCompiler.arrayLength(*this, array);
                break;
            }

            case OpCodeEnum::ATHROW: {
                const auto ex = popValue();
                methodCompiler.throwException(*this, ex);
                break;
            }

            case OpCodeEnum::CHECKCAST: {
                const auto index = byteReader.readU2();
                const auto ref = topValue();
                methodCompiler.checkCast(index, ref);
                break;
            }

            case OpCodeEnum::INSTANCEOF: {
                const auto index = byteReader.readU2();
                const auto ref = popValue();
                methodCompiler.instanceOf(*this, index, ref);
                break;
            }

            case OpCodeEnum::MONITORENTER:
            case OpCodeEnum::MONITOREXIT: {
                const u1 type = CAST_I4(opCode) - CAST_I4(OpCodeEnum::MONITORENTER);
                const auto oop = popValue();
                methodCompiler.monitor(*this, type, oop);
                break;
            }

            case OpCodeEnum::WIDE: {
                const auto wideOpCode = static_cast<OpCodeEnum>(byteReader.readU1());
                const auto index = byteReader.readU2();

                switch (wideOpCode) {
                    case OpCodeEnum::ILOAD:
                        methodCompiler.load(*this, index, SlotTypeEnum::I4);
                        break;
                    case OpCodeEnum::FLOAD:
                        methodCompiler.load(*this, index, SlotTypeEnum::F4);
                        break;
                    case OpCodeEnum::ALOAD:
                        methodCompiler.load(*this, index, SlotTypeEnum::REF);
                        break;
                    case OpCodeEnum::LLOAD:
                        methodCompiler.load(*this, index, SlotTypeEnum::I8);
                        break;
                    case OpCodeEnum::DLOAD:
                        methodCompiler.load(*this, index, SlotTypeEnum::F8);
                        break;

                    case OpCodeEnum::ISTORE:
                        methodCompiler.store(*this, index, SlotTypeEnum::I4);
                        break;
                    case OpCodeEnum::FSTORE:
                        methodCompiler.store(*this, index, SlotTypeEnum::F4);
                        break;
                    case OpCodeEnum::ASTORE:
                        methodCompiler.store(*this, index, SlotTypeEnum::REF);
                        break;
                    case OpCodeEnum::LSTORE:
                        methodCompiler.store(*this, index, SlotTypeEnum::I8);
                        break;
                    case OpCodeEnum::DSTORE:
                        methodCompiler.store(*this, index, SlotTypeEnum::F8);
                        break;

                    case OpCodeEnum::IINC: {
                        const auto value = byteReader.readI2();
                        const auto currentValue = methodCompiler.getLocalVariableTableValue(index, SlotTypeEnum::I4);
                        const auto incValue = irBuilder.CreateAdd(currentValue, irBuilder.getInt32(value));
                        methodCompiler.setLocalVariableTableValue(index, incValue, SlotTypeEnum::I4);
                        break;
                    }

                    case OpCodeEnum::RET:
                        panic("ret not implement!");
                    break;

                    default:
                        panic("wide error");
                }
                break;
            }

            case OpCodeEnum::MULTIANEWARRAY: {
                const auto index = byteReader.readU2();
                const auto dimension = byteReader.readU1();
                const auto arrayType = llvm::ArrayType::get(irBuilder.getInt32Ty(), dimension);
                const auto arrayValue = irBuilder.CreateAlloca(arrayType);
                for (i4 i = dimension - 1; i >= 0; --i) {
                    const auto dimValue = popValue();
                    const auto dimPtr =
                        irBuilder.CreateGEP(irBuilder.getInt32Ty(), arrayValue, irBuilder.getInt32(i));
                    irBuilder.CreateStore(dimValue, dimPtr);
                }
                methodCompiler.newMultiArray(*this, index, dimension, arrayValue);
                break;
            }

            case OpCodeEnum::IFNULL:
            case OpCodeEnum::IFNONNULL: {
                const auto curOp = CAST_U1(opCode) - (static_cast<u1>(OpCodeEnum::IFNULL) - static_cast<u1>(OpCodeEnum::IFEQ));
                const auto offset = byteReader.readI2();
                const auto jumpTo = CAST_U4(CAST_I4(pc) + offset);
                const auto val = popValue();
                methodCompiler.ifOp(*this, offset, val, methodCompiler.getZeroValue(SlotTypeEnum::REF), static_cast<OpCodeEnum>(curOp));
                break;
            }

            case OpCodeEnum::GOTO_W: {
                const auto offset = byteReader.readI4();
                methodCompiler.jumpTo(*this, offset);
                break;
            }

            default:
                panic("not support");
                break;
        }
    }

}
