#include "method_utils.hpp"
#include <tuple>
#include "../class_member.hpp"
#include "../opcode.hpp"
#include "byte_reader.hpp"

namespace RexVM {

    std::vector<std::tuple<u4, u4>> scanMethodLabel(const Method &method) {
        ByteReader reader{};
        const auto codePtr = method.code.get();
        reader.init(codePtr, method.codeLength);

        std::vector<std::tuple<u4, u4>> labels;

        const auto addLabel = [&](const u4 pc, const i4 offset) {
            const auto label = std::make_tuple(pc, CAST_U4(CAST_I4(pc) + offset));
            labels.emplace_back(label);
        };

        while (!reader.eof()) {
            const auto currentByteCode = reader.readU1();
            const auto pc = CAST_U4(reader.ptr - reader.begin) - 1;
            switch (static_cast<OpCodeEnum>(currentByteCode)) {
                // case OpCodeEnum::NOP:
                // case OpCodeEnum::ACONST_NULL:
                // case OpCodeEnum::ICONST_M1:
                // case OpCodeEnum::ICONST_0:
                // case OpCodeEnum::ICONST_1:
                // case OpCodeEnum::ICONST_2:
                // case OpCodeEnum::ICONST_3:
                // case OpCodeEnum::ICONST_4:
                // case OpCodeEnum::ICONST_5:
                // case OpCodeEnum::LCONST_0:
                // case OpCodeEnum::LCONST_1:
                // case OpCodeEnum::FCONST_0:
                // case OpCodeEnum::FCONST_1:
                // case OpCodeEnum::FCONST_2:
                // case OpCodeEnum::DCONST_0:
                // case OpCodeEnum::DCONST_1:
                //     break;

                case OpCodeEnum::BIPUSH:
                    reader.readI1();
                    break;
                case OpCodeEnum::SIPUSH:
                    reader.readI2();
                    break;

                case OpCodeEnum::LDC:
                    reader.readU1();
                    break;
                case OpCodeEnum::LDC_W:
                case OpCodeEnum::LDC2_W:
                    reader.readU2();
                    break;

                case OpCodeEnum::ILOAD:
                case OpCodeEnum::LLOAD:
                case OpCodeEnum::FLOAD:
                case OpCodeEnum::DLOAD:
                case OpCodeEnum::ALOAD:
                    reader.readU1();
                    break;

                // case OpCodeEnum::ILOAD_0:
                // case OpCodeEnum::ILOAD_1:
                // case OpCodeEnum::ILOAD_2:
                // case OpCodeEnum::ILOAD_3:
                // case OpCodeEnum::LLOAD_0:
                // case OpCodeEnum::LLOAD_1:
                // case OpCodeEnum::LLOAD_2:
                // case OpCodeEnum::LLOAD_3:
                // case OpCodeEnum::FLOAD_0:
                // case OpCodeEnum::FLOAD_1:
                // case OpCodeEnum::FLOAD_2:
                // case OpCodeEnum::FLOAD_3:
                // case OpCodeEnum::DLOAD_0:
                // case OpCodeEnum::DLOAD_1:
                // case OpCodeEnum::DLOAD_2:
                // case OpCodeEnum::DLOAD_3:
                // case OpCodeEnum::ALOAD_0:
                // case OpCodeEnum::ALOAD_1:
                // case OpCodeEnum::ALOAD_2:
                // case OpCodeEnum::ALOAD_3:
                // case OpCodeEnum::IALOAD:
                // case OpCodeEnum::LALOAD:
                // case OpCodeEnum::FALOAD:
                // case OpCodeEnum::DALOAD:
                // case OpCodeEnum::AALOAD:
                // case OpCodeEnum::BALOAD:
                // case OpCodeEnum::CALOAD:
                // case OpCodeEnum::SALOAD:
                //     break;

                case OpCodeEnum::ISTORE:
                case OpCodeEnum::LSTORE:
                case OpCodeEnum::FSTORE:
                case OpCodeEnum::DSTORE:
                case OpCodeEnum::ASTORE:
                    reader.readU1();
                    break;

                // case OpCodeEnum::ISTORE_0:
                // case OpCodeEnum::ISTORE_1:
                // case OpCodeEnum::ISTORE_2:
                // case OpCodeEnum::ISTORE_3:
                // case OpCodeEnum::LSTORE_0:
                // case OpCodeEnum::LSTORE_1:
                // case OpCodeEnum::LSTORE_2:
                // case OpCodeEnum::LSTORE_3:
                // case OpCodeEnum::FSTORE_0:
                // case OpCodeEnum::FSTORE_1:
                // case OpCodeEnum::FSTORE_2:
                // case OpCodeEnum::FSTORE_3:
                // case OpCodeEnum::DSTORE_0:
                // case OpCodeEnum::DSTORE_1:
                // case OpCodeEnum::DSTORE_2:
                // case OpCodeEnum::DSTORE_3:
                // case OpCodeEnum::ASTORE_0:
                // case OpCodeEnum::ASTORE_1:
                // case OpCodeEnum::ASTORE_2:
                // case OpCodeEnum::ASTORE_3:
                // case OpCodeEnum::IASTORE:
                // case OpCodeEnum::LASTORE:
                // case OpCodeEnum::FASTORE:
                // case OpCodeEnum::DASTORE:
                // case OpCodeEnum::AASTORE:
                // case OpCodeEnum::BASTORE:
                // case OpCodeEnum::CASTORE:
                // case OpCodeEnum::SASTORE:
                // case OpCodeEnum::POP:
                // case OpCodeEnum::POP2:
                // case OpCodeEnum::DUP:
                // case OpCodeEnum::DUP_X1:
                // case OpCodeEnum::DUP_X2:
                // case OpCodeEnum::DUP2:
                // case OpCodeEnum::DUP2_X1:
                // case OpCodeEnum::DUP2_X2:
                // case OpCodeEnum::SWAP:
                // case OpCodeEnum::IADD:
                // case OpCodeEnum::LADD:
                // case OpCodeEnum::FADD:
                // case OpCodeEnum::DADD:
                // case OpCodeEnum::ISUB:
                // case OpCodeEnum::LSUB:
                // case OpCodeEnum::FSUB:
                // case OpCodeEnum::DSUB:
                // case OpCodeEnum::IMUL:
                // case OpCodeEnum::LMUL:
                // case OpCodeEnum::FMUL:
                // case OpCodeEnum::DMUL:
                // case OpCodeEnum::IDIV:
                // case OpCodeEnum::LDIV:
                // case OpCodeEnum::FDIV:
                // case OpCodeEnum::DDIV:
                // case OpCodeEnum::IREM:
                // case OpCodeEnum::LREM:
                // case OpCodeEnum::FREM:
                // case OpCodeEnum::DREM:
                // case OpCodeEnum::INEG:
                // case OpCodeEnum::LNEG:
                // case OpCodeEnum::FNEG:
                // case OpCodeEnum::DNEG:
                // case OpCodeEnum::ISHL:
                // case OpCodeEnum::LSHL:
                // case OpCodeEnum::ISHR:
                // case OpCodeEnum::LSHR:
                // case OpCodeEnum::IUSHR:
                // case OpCodeEnum::LUSHR:
                // case OpCodeEnum::IAND:
                // case OpCodeEnum::LAND:
                // case OpCodeEnum::IOR:
                // case OpCodeEnum::LOR:
                // case OpCodeEnum::IXOR:
                // case OpCodeEnum::LXOR:
                //     break;

                case OpCodeEnum::IINC:
                    reader.readU1();
                    reader.readI1();
                    break;

                // case OpCodeEnum::I2L:
                // case OpCodeEnum::I2F:
                // case OpCodeEnum::I2D:
                // case OpCodeEnum::L2I:
                // case OpCodeEnum::L2F:
                // case OpCodeEnum::L2D:
                // case OpCodeEnum::F2I:
                // case OpCodeEnum::F2L:
                // case OpCodeEnum::F2D:
                // case OpCodeEnum::D2I:
                // case OpCodeEnum::D2L:
                // case OpCodeEnum::D2F:
                // case OpCodeEnum::I2B:
                // case OpCodeEnum::I2C:
                // case OpCodeEnum::I2S:
                // case OpCodeEnum::LCMP:
                // case OpCodeEnum::FCMPL:
                // case OpCodeEnum::FCMPG:
                // case OpCodeEnum::DCMPL:
                // case OpCodeEnum::DCMPG:
                //     break;

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
                case OpCodeEnum::IF_ACMPNE:
                case OpCodeEnum::GOTO: {
                    const auto offset = reader.readI2();
                    addLabel(pc, offset);
                    break;
                }

                // case OpCodeEnum::JSR:
                // case OpCodeEnum::RET:
                //     break;

                case OpCodeEnum::TABLESWITCH: {
                    const auto nextPc = CAST_U4(reader.ptr - reader.begin);
                    if (const auto mod = nextPc % 4; mod != 0) {
                        reader.skip(4 - mod);
                    }
                    const auto defaultOffset = reader.readI4();

                    const auto low = reader.readI4();
                    const auto high = reader.readI4();
                    const auto jumpOffsetsCount = high - low + 1;
                    auto jumpOffsets = std::make_unique<i4[]>(jumpOffsetsCount);
                    for (i4 i = 0; i < jumpOffsetsCount; ++i) {
                        jumpOffsets[i] = reader.readI4();
                        addLabel(pc, jumpOffsets[i]);
                    }

                    addLabel(pc, defaultOffset);

                    break;
                }

                case OpCodeEnum::LOOKUPSWITCH: {
                    const auto nextPc = CAST_U4(reader.ptr - reader.begin);
                    if (const auto mod = nextPc % 4; mod != 0) {
                        reader.skip(4 - mod);
                    }
                    const auto defaultOffset = reader.readI4();

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

                    addLabel(pc, defaultOffset);

                    break;
                }

                // case OpCodeEnum::IRETURN:
                // case OpCodeEnum::LRETURN:
                // case OpCodeEnum::FRETURN:
                // case OpCodeEnum::DRETURN:
                // case OpCodeEnum::ARETURN:
                // case OpCodeEnum::RETURN:
                //     break;

                case OpCodeEnum::GETSTATIC:
                case OpCodeEnum::PUTSTATIC:
                case OpCodeEnum::GETFIELD:
                case OpCodeEnum::PUTFIELD:
                    reader.readU2();
                    break;

                case OpCodeEnum::INVOKEVIRTUAL:
                case OpCodeEnum::INVOKESPECIAL:
                case OpCodeEnum::INVOKESTATIC:
                    reader.readU2();
                    break;

                case OpCodeEnum::INVOKEINTERFACE:
                case OpCodeEnum::INVOKEDYNAMIC:
                    reader.readU2();
                    reader.readU2();
                    break;

                case OpCodeEnum::NEW:
                    reader.readU2();
                    break;

                case OpCodeEnum::NEWARRAY:
                    reader.readU1();
                    break;

                case OpCodeEnum::ANEWARRAY:
                    reader.readU2();
                    break;

                case OpCodeEnum::ARRAYLENGTH:
                case OpCodeEnum::ATHROW:
                    break;

                case OpCodeEnum::CHECKCAST:
                case OpCodeEnum::INSTANCEOF:
                    reader.readU2();
                    break;

                case OpCodeEnum::MONITORENTER:
                case OpCodeEnum::MONITOREXIT:
                    break;

                case OpCodeEnum::WIDE: {
                    const auto opCodeWide = static_cast<OpCodeEnum>(reader.readU1());
                    const auto index = reader.readU2();

                    if (opCodeWide == OpCodeEnum::IINC) {
                        reader.readI2();
                    }

                    break;
                }
                case OpCodeEnum::MULTIANEWARRAY:
                    reader.readU2();
                    reader.readU1();
                    break;

                case OpCodeEnum::IFNULL:
                case OpCodeEnum::IFNONNULL: {
                    const auto offset = reader.readI2();
                    addLabel(pc, offset);
                    break;
                }

                case OpCodeEnum::GOTO_W: {
                    const auto offset = reader.readI4();
                    addLabel(pc, offset);
                    break;
                }
                
                default:
                    break;
            }
        }
        return labels;
    }

}