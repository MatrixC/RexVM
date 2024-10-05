#include "cfg.hpp"

#include <llvm/ExecutionEngine/JITLink/JITLink.h>

#include "class_member.hpp"
#include "opcode.hpp"
#include "utils/byte_reader.hpp"

namespace RexVM {
    MethodBlock::MethodBlock(const Method &method, const u4 startPC, const u4 endPC)
        : length(endPC - startPC),
          startPC(startPC),
          endPC(endPC) {
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
        std::vector<u8> edges;

        const auto getPC = [&] {
            return CAST_U4(reader.ptr - reader.begin);
        };

        const auto addLabel = [&](const u4 pc, const i4 offset) {
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

        while (!reader.eof()) {
            const auto pc = getPC();
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

                default:
                    break;
            }
        }

        std::ranges::sort(leaders);
        leaders.erase(std::ranges::unique(leaders).begin(), leaders.end());

        std::ranges::sort(edges);
        edges.erase(std::ranges::unique(edges).begin(), edges.end());

        u4 lastBlockStartPC = 0;
        for (const u4 leaderPC: leaders) {
            blocks.emplace_back(std::make_unique<MethodBlock>(method, lastBlockStartPC, leaderPC));
            lastBlockStartPC = leaderPC;
        }
        blocks.emplace_back(std::make_unique<MethodBlock>(method, lastBlockStartPC, method.codeLength));

        for (const u8 edge: edges) {
            const auto fromPC = edge >> 32 & 0xFFFFFFFF;
            const auto jumpTo = edge & 0xFFFFFFFF;

            const auto fromPCIndex = std::distance(leaders.begin(), std::ranges::upper_bound(leaders, fromPC));
            const auto jumpToIndex = std::distance(leaders.begin(), std::ranges::upper_bound(leaders, jumpTo));

            blocks[fromPCIndex]->jumpToBlockIndex.emplace_back(jumpToIndex);
            blocks[jumpToIndex]->parentBlockIndex.emplace_back(fromPCIndex);
        }

        for (size_t i = 0; i < blocks.size() - 1; ++i) {
            if (const auto block = blocks[i].get(); block->jumpToBlockIndex.empty()) {
                block->autoJmp = true;
                const auto jumpToIdx = i + 1;
                const auto nextBlock = blocks[jumpToIdx].get();
                block->jumpToBlockIndex.emplace_back(jumpToIdx);
                nextBlock->parentBlockIndex.emplace_back(i);
            }
        }
    }
}
