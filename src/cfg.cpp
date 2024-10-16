#include "cfg.hpp"

#include "class_member.hpp"
#include "opcode.hpp"
#include "utils/byte_reader.hpp"
#include "class_member.hpp"
#include "class.hpp"
#include "class_loader.hpp"
#include "constant_info.hpp"


namespace RexVM {
    MethodBlock::MethodBlock(const u4 index, const u4 startPC, const u4 endPC, const u4 lastPC)
        : index(index),
          length(endPC - startPC),
          startPC(startPC),
          endPC(endPC),
          lastPC(lastPC) {
        //[startPC, endPC)
    }

    MethodCFG::MethodCFG(const Method &method) : method(method) {
        build();
    }

    void MethodCFG::build() {
        ByteReader reader{};
        const auto codePtr = method.code.get();
        reader.init(codePtr, method.codeLength);

        std::vector<u4> leaders;
        leaders.reserve(40);

        leaders.emplace_back(0);
        std::vector<u8> edges;

        if (!method.exceptionCatches.empty()) {
            for (const auto &exception : method.exceptionCatches) {
                const auto handlerPC = exception->handler;
                leaders.emplace_back(handlerPC);
            }
        }

        const auto getPC = [&] {
            return CAST_U4(reader.ptr - reader.begin);
        };

        const auto addLabel = [&](const u4 pc, const i4 offset) {
            if (offset < 0) {
                this->jumpFront = true;
            }
            const auto jumpTo = CAST_U4(CAST_I4(pc) + offset);
            leaders.emplace_back(jumpTo);
            const auto edge = (CAST_U8(pc) << 32) | jumpTo;
            edges.emplace_back(edge);
        };

        const auto addNextPCLabel = [&](const u4 pc) {
            const auto jumpTo = getPC();
            leaders.emplace_back(getPC());
            const auto edge = (CAST_U8(pc) << 32) | jumpTo;
            edges.emplace_back(edge);
        };

        std::vector<u4> pcCodes; //记录所有PC 用于查找每个块的最后一个pc
        pcCodes.reserve(200);

        std::vector<u4> returnMethodPC; //记录return指令的pc 用户计算autoJump
        pcCodes.reserve(20);

        while (!reader.eof()) {
            const auto pc = getPC();
            pcCodes.emplace_back(pc);
            const auto currentByteCode = reader.readU1();
            const auto opCode = static_cast<OpCodeEnum>(currentByteCode);
            switch (opCode) {
                case OpCodeEnum::IFEQ:
                case OpCodeEnum::IFNE:
                case OpCodeEnum::IFLT:
                case OpCodeEnum::IFGE:
                case OpCodeEnum::IFGT:
                case OpCodeEnum::IFLE:
                case OpCodeEnum::IF_ICMPEQ:
                case OpCodeEnum::IF_ICMPNE:
                case OpCodeEnum::IF_ICMPLT:
                case OpCodeEnum::IF_ICMPGE:
                case OpCodeEnum::IF_ICMPGT:
                case OpCodeEnum::IF_ICMPLE:
                case OpCodeEnum::IF_ACMPEQ:
                case OpCodeEnum::IF_ACMPNE: {
                    const auto offset = reader.readI2();
                    addLabel(pc, offset);
                    addNextPCLabel(pc);
                    break;
                }
                case OpCodeEnum::GOTO: {
                    const auto offset = reader.readI2();
                    addLabel(pc, offset);
                    break;
                }

                case OpCodeEnum::TABLESWITCH: {
                    const auto nextPc = CAST_U4(reader.ptr - reader.begin);
                    if (const auto mod = nextPc % 4; mod != 0) {
                        reader.skip(4 - mod);
                    }
                    const auto defaultOffset = reader.readI4();
                    addLabel(pc, defaultOffset);
                    const auto low = reader.readI4();
                    const auto high = reader.readI4();
                    const auto jumpOffsetsCount = high - low + 1;
                    auto jumpOffsets = std::make_unique<i4[]>(jumpOffsetsCount);
                    for (i4 i = 0; i < jumpOffsetsCount; ++i) {
                        jumpOffsets[i] = reader.readI4();
                        addLabel(pc, jumpOffsets[i]);
                    }

                    addNextPCLabel(pc);
                    break;
                }

                case OpCodeEnum::LOOKUPSWITCH: {
                    const auto nextPc = CAST_U4(reader.ptr - reader.begin);
                    if (const auto mod = nextPc % 4; mod != 0) {
                        reader.skip(4 - mod);
                    }
                    const auto defaultOffset = reader.readI4();
                    addLabel(pc, defaultOffset);
                    const auto npairs = reader.readI4();
                    const auto count = npairs * 2;
                    auto matchOffsets = std::make_unique<i4[]>(count);
                    //matchOffsets[0] = key, matchOffsets[1] = offset
                    //matchOffsets[2] = key, matchOffsets[3] = offset

                    for (i4 i = 0; i < count; ++i) {
                        matchOffsets[i] = reader.readI4();
                        if (i % 2 != 0) {
                            addLabel(pc, matchOffsets[i]);
                        }
                    }

                    addNextPCLabel(pc);
                    break;
                }

                case OpCodeEnum::IFNULL:
                case OpCodeEnum::IFNONNULL: {
                    const auto offset = reader.readI2();
                    addLabel(pc, offset);
                    addNextPCLabel(pc);
                    break;
                }

                case OpCodeEnum::GOTO_W: {
                    const auto offset = reader.readI4();
                    addLabel(pc, offset);
                    break;
                }

                case OpCodeEnum::BIPUSH:
                case OpCodeEnum::LDC:
                case OpCodeEnum::ILOAD:
                case OpCodeEnum::LLOAD:
                case OpCodeEnum::FLOAD:
                case OpCodeEnum::DLOAD:
                case OpCodeEnum::ALOAD:
                case OpCodeEnum::ISTORE:
                case OpCodeEnum::LSTORE:
                case OpCodeEnum::FSTORE:
                case OpCodeEnum::DSTORE:
                case OpCodeEnum::ASTORE:
                case OpCodeEnum::NEWARRAY:
                    reader.skip(1);
                    break;

                case OpCodeEnum::SIPUSH:
                case OpCodeEnum::LDC_W:
                case OpCodeEnum::LDC2_W:
                case OpCodeEnum::IINC:
                case OpCodeEnum::GETSTATIC:
                case OpCodeEnum::PUTSTATIC:
                case OpCodeEnum::GETFIELD:
                case OpCodeEnum::PUTFIELD:
                case OpCodeEnum::INVOKEVIRTUAL:
                case OpCodeEnum::INVOKESPECIAL:
                case OpCodeEnum::INVOKESTATIC:
                case OpCodeEnum::NEW:
                case OpCodeEnum::ANEWARRAY:
                case OpCodeEnum::CHECKCAST:
                case OpCodeEnum::INSTANCEOF:
                    reader.skip(2);
                    break;

                case OpCodeEnum::MULTIANEWARRAY:
                    reader.skip(3);
                    break;

                case OpCodeEnum::INVOKEINTERFACE:
                case OpCodeEnum::INVOKEDYNAMIC:
                    reader.skip(4);
                    break;

                case OpCodeEnum::WIDE: {
                    const auto opCodeWide = static_cast<OpCodeEnum>(reader.readU1());
                    reader.skip(2);
                    if (opCodeWide == OpCodeEnum::IINC) {
                        reader.skip(2);
                    }
                    break;
                }

                case OpCodeEnum::IRETURN:
                case OpCodeEnum::LRETURN:
                case OpCodeEnum::FRETURN:
                case OpCodeEnum::DRETURN:
                case OpCodeEnum::ARETURN:
                case OpCodeEnum::RETURN:
                case OpCodeEnum::ATHROW:
                    returnMethodPC.emplace_back(pc);
                    break;

                default:
                    break;
            }
        }

        std::ranges::sort(leaders);
        leaders.erase(std::ranges::unique(leaders).begin(), leaders.end());

        std::ranges::sort(edges);
        edges.erase(std::ranges::unique(edges).begin(), edges.end());

        u4 methodBlockIndex{0};
        const auto leadersSize = leaders.size();
        for (size_t i = 0; i < leadersSize; ++i) {
            const auto startPC = leaders[i];
            //最后一个块的endPC = method.codeLength
            const auto endPC = i < leadersSize - 1 ? leaders[i + 1] : method.codeLength;

            //在pcCode中找到endPC 因为它是下一个块的起始opCodePC 所以取前一个 就是当前块lastPC
            const auto endPCIdx = std::distance(pcCodes.begin(),  std::ranges::lower_bound(pcCodes, endPC));
            const auto lastPC = endPCIdx == 0 ? 0 : pcCodes[endPCIdx - 1];

            blocks.emplace_back(std::make_unique<MethodBlock>(methodBlockIndex++, startPC, endPC, lastPC));
        }

        if (!method.exceptionCatches.empty()) {
            //添加异常
            for (const auto &exception : method.exceptionCatches) {
                const auto handlerPC = exception->handler;
                const auto handleBlockIndex = std::distance(leaders.begin(), std::ranges::lower_bound(leaders, handlerPC));
                const auto handleMethodBlock = blocks[handleBlockIndex].get();
                handleMethodBlock->exceptionHandlerBlock = true;
                handleMethodBlock->catchStartPC = exception->start;
                handleMethodBlock->catchEndPC = exception->end;
                if (exception->catchType != 0) {
                    if (exception->catchClass == nullptr) {
                        const auto &constantPool = method.klass.constantPool;
                        const auto exClassName =
                                getConstantStringFromPoolByIndexInfo(constantPool, exception->catchType);
                        exception->catchClass = method.klass.classLoader.getInstanceClass(exClassName);
                    }
                    handleMethodBlock->catchClass = exception->catchClass;
                }
            }
        }

        for (const u8 edge: edges) {
            const auto fromPC = edge >> 32 & 0xFFFFFFFF;
            const auto jumpTo = edge & 0xFFFFFFFF;

            const auto fromPCIndex = std::distance(leaders.begin(), std::ranges::upper_bound(leaders, fromPC)) - 1;
            const auto jumpToIndex = std::distance(leaders.begin(), std::ranges::upper_bound(leaders, jumpTo)) - 1;

            blocks[fromPCIndex]->jumpToBlockIndex.emplace_back(jumpToIndex);
            blocks[jumpToIndex]->parentBlockIndex.emplace_back(fromPCIndex);
        }

        for (size_t i = 0; i < blocks.size() - 1; ++i) {
            if (const auto block = blocks[i].get(); block->jumpToBlockIndex.empty()) {
                // const auto lastOpCodePC = block->endPC - 1;
                const auto lastPC = block->lastPC;
                if (const auto iter = std::ranges::find(returnMethodPC, lastPC); iter == returnMethodPC.end()) {
                    //如果发现一个块 没有jumpTo 且它的最后一个opCode不是return 则它需要autoJump到下一个块
                    block->autoJmp = true;
                    const auto jumpToIdx = i + 1;
                    block->jumpToBlockIndex.emplace_back(jumpToIdx);
                    const auto nextBlock = blocks[jumpToIdx].get();
                    nextBlock->parentBlockIndex.emplace_back(i);
                }
            }
        }
    }
}
