#include "opcode.hpp"
#include "config.hpp"

namespace RexVM {
    const std::array<cview, OPCODE_ENUM_COUNT> OPCODE_NAMES = {
        "nop",
        "aconst_null",
        "iconst_m1",
        "iconst_0",
        "iconst_1",
        "iconst_2",
        "iconst_3",
        "iconst_4",
        "iconst_5",
        "lconst_0",
        "lconst_1",
        "fconst_0",
        "fconst_1",
        "fconst_2",
        "dconst_0",
        "dconst_1",
        "bipush",
        "sipush",
        "ldc",
        "ldc_w",
        "ldc2_w",
        "iload",
        "lload",
        "fload",
        "dload",
        "aload",
        "iload_0",
        "iload_1",
        "iload_2",
        "iload_3",
        "lload_0",
        "lload_1",
        "lload_2",
        "lload_3",
        "fload_0",
        "fload_1",
        "fload_2",
        "fload_3",
        "dload_0",
        "dload_1",
        "dload_2",
        "dload_3",
        "aload_0",
        "aload_1",
        "aload_2",
        "aload_3",
        "iaload",
        "laload",
        "faload",
        "daload",
        "aaload",
        "baload",
        "caload",
        "saload",
        "istore",
        "lstore",
        "fstore",
        "dstore",
        "astore",
        "istore_0",
        "istore_1",
        "istore_2",
        "istore_3",
        "lstore_0",
        "lstore_1",
        "lstore_2",
        "lstore_3",
        "fstore_0",
        "fstore_1",
        "fstore_2",
        "fstore_3",
        "dstore_0",
        "dstore_1",
        "dstore_2",
        "dstore_3",
        "astore_0",
        "astore_1",
        "astore_2",
        "astore_3",
        "iastore",
        "lastore",
        "fastore",
        "dastore",
        "aastore",
        "bastore",
        "castore",
        "sastore",
        "pop",
        "pop2",
        "dup",
        "dup_x1",
        "dup_x2",
        "dup2",
        "dup2_x1",
        "dup2_x2",
        "swap",
        "iadd",
        "ladd",
        "fadd",
        "dadd",
        "isub",
        "lsub",
        "fsub",
        "dsub",
        "imul",
        "lmul",
        "fmul",
        "dmul",
        "idiv",
        "ldiv",
        "fdiv",
        "ddiv",
        "irem",
        "lrem",
        "frem",
        "drem",
        "ineg",
        "lneg",
        "fneg",
        "dneg",
        "ishl",
        "lshl",
        "ishr",
        "lshr",
        "iushr",
        "lushr",
        "iand",
        "land",
        "ior",
        "lor",
        "ixor",
        "lxor",
        "iinc",
        "i2l",
        "i2f",
        "i2d",
        "l2i",
        "l2f",
        "l2d",
        "f2i",
        "f2l",
        "f2d",
        "d2i",
        "d2l",
        "d2f",
        "i2b",
        "i2c",
        "i2s",
        "lcmp",
        "fcmpl",
        "fcmpg",
        "dcmpl",
        "dcmpg",
        "ifeq",
        "ifne",
        "iflt",
        "ifge",
        "ifgt",
        "ifle",
        "if_icmpeq",
        "if_icmpne",
        "if_icmplt",
        "if_icmpge",
        "if_icmpgt",
        "if_icmple",
        "if_acmpeq",
        "if_acmpne",
        "goto",
        "jsr",
        "ret",
        "tableswitch",
        "lookupswitch",
        "ireturn",
        "lreturn",
        "freturn",
        "dreturn",
        "areturn",
        "return",
        "getstatic",
        "putstatic",
        "getfield",
        "putfield",
        "invokevirtual",
        "invokespecial",
        "invokestatic",
        "invokeinterface",
        "invokedynamic",
        "new",
        "newarray",
        "anewarray",
        "arraylength",
        "athrow",
        "checkcast",
        "instanceof",
        "monitorenter",
        "monitorexit",
        "wide",
        "multianewarray",
        "ifnull",
        "ifnonnull",
        "goto_w",
        "jsr_w",
        "breakpoint",
        "ldc_quick",
        "ldc_w_quick",
        "ldc2_w_quick",
        "getfield_quick",
        "putfield_quick",
        "getfield2_quick",
        "putfield2_quick",
        "getstatic_quick",
        "putstatic_quick",
        "getstatic2_quick",
        "putstatic2_quick",
        "invokevirtual_quick",
        "invokenonvirtual_quick",
        "invokesuper_quick",
        "invokestatic_quick",
        "invokeinterface_quick",
        "invokevirtualobject_quick",
        "new_quick",
        "anewarray_quick",
        "multianewarray_quick",
        "checkcast_quick",
        "instanceof_quick",
        "invokevirtual_quick_w",
        "getfield_quick_w",
        "putfield_quick_w",
        "impdep1",
        "impdep2"
    };

    cview getOpCodeName(OpCodeEnum opCode) {
        return OPCODE_NAMES[static_cast<i4>(opCode)];
    }

    bool isJumpOpCode(const OpCodeEnum opCode) {
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
            case OpCodeEnum::IF_ACMPNE:
            case OpCodeEnum::GOTO:
            case OpCodeEnum::TABLESWITCH:
            case OpCodeEnum::LOOKUPSWITCH:
            case OpCodeEnum::IFNULL:
            case OpCodeEnum::IFNONNULL:
            case OpCodeEnum::GOTO_W:
                return true;
            default:
                return false;
        }
    }
    bool isReturnOpCode(const OpCodeEnum opCode) {
        switch (opCode) {
            case OpCodeEnum::IRETURN:
            case OpCodeEnum::LRETURN:
            case OpCodeEnum::FRETURN:
            case OpCodeEnum::DRETURN:
            case OpCodeEnum::ARETURN:
            case OpCodeEnum::RETURN:
                return true;
            default:
                return false;
        }
    }

    bool isEndOpCode(const OpCodeEnum opCode) {
        return isJumpOpCode(opCode) || isReturnOpCode(opCode);
    }

}
