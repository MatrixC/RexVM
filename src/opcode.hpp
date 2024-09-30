#ifndef OPCODE_HPP
#define OPCODE_HPP

#include "config.hpp"
#include <memory>

namespace RexVM {
    enum class OpCodeEnum : u1 {

        NOP = 0,

        ACONST_NULL = 1,

        ICONST_M1 = 2,

        ICONST_0 = 3,

        ICONST_1 = 4,

        ICONST_2 = 5,

        ICONST_3 = 6,

        ICONST_4 = 7,

        ICONST_5 = 8,

        LCONST_0 = 9,

        LCONST_1 = 10,

        FCONST_0 = 11,

        FCONST_1 = 12,

        FCONST_2 = 13,

        DCONST_0 = 14,

        DCONST_1 = 15,

        BIPUSH = 16,

        SIPUSH = 17,

        LDC = 18,

        LDC_W = 19,

        LDC2_W = 20,

        ILOAD = 21,

        LLOAD = 22,

        FLOAD = 23,

        DLOAD = 24,

        ALOAD = 25,

        ILOAD_0 = 26,

        ILOAD_1 = 27,

        ILOAD_2 = 28,

        ILOAD_3 = 29,

        LLOAD_0 = 30,

        LLOAD_1 = 31,

        LLOAD_2 = 32,

        LLOAD_3 = 33,

        FLOAD_0 = 34,

        FLOAD_1 = 35,

        FLOAD_2 = 36,

        FLOAD_3 = 37,

        DLOAD_0 = 38,

        DLOAD_1 = 39,

        DLOAD_2 = 40,

        DLOAD_3 = 41,

        ALOAD_0 = 42,

        ALOAD_1 = 43,

        ALOAD_2 = 44,

        ALOAD_3 = 45,

        IALOAD = 46,

        LALOAD = 47,

        FALOAD = 48,

        DALOAD = 49,

        AALOAD = 50,

        BALOAD = 51,

        CALOAD = 52,

        SALOAD = 53,

        ISTORE = 54,

        LSTORE = 55,

        FSTORE = 56,

        DSTORE = 57,

        ASTORE = 58,

        ISTORE_0 = 59,

        ISTORE_1 = 60,

        ISTORE_2 = 61,

        ISTORE_3 = 62,

        LSTORE_0 = 63,

        LSTORE_1 = 64,

        LSTORE_2 = 65,

        LSTORE_3 = 66,

        FSTORE_0 = 67,

        FSTORE_1 = 68,

        FSTORE_2 = 69,

        FSTORE_3 = 70,

        DSTORE_0 = 71,

        DSTORE_1 = 72,

        DSTORE_2 = 73,

        DSTORE_3 = 74,

        ASTORE_0 = 75,

        ASTORE_1 = 76,

        ASTORE_2 = 77,

        ASTORE_3 = 78,

        IASTORE = 79,

        LASTORE = 80,

        FASTORE = 81,

        DASTORE = 82,

        AASTORE = 83,

        BASTORE = 84,

        CASTORE = 85,

        SASTORE = 86,

        POP = 87,

        POP2 = 88,

        DUP = 89,

        DUP_X1 = 90,

        DUP_X2 = 91,

        DUP2 = 92,

        DUP2_X1 = 93,

        DUP2_X2 = 94,

        SWAP = 95,

        IADD = 96,

        LADD = 97,

        FADD = 98,

        DADD = 99,

        ISUB = 100,

        LSUB = 101,

        FSUB = 102,

        DSUB = 103,

        IMUL = 104,

        LMUL = 105,

        FMUL = 106,

        DMUL = 107,

        IDIV = 108,

        LDIV = 109,

        FDIV = 110,

        DDIV = 111,

        IREM = 112,

        LREM = 113,

        FREM = 114,

        DREM = 115,

        INEG = 116,

        LNEG = 117,

        FNEG = 118,

        DNEG = 119,

        ISHL = 120,

        LSHL = 121,

        ISHR = 122,

        LSHR = 123,

        IUSHR = 124,

        LUSHR = 125,

        IAND = 126,

        LAND = 127,

        IOR = 128,

        LOR = 129,

        IXOR = 130,

        LXOR = 131,

        IINC = 132,

        I2L = 133,

        I2F = 134,

        I2D = 135,

        L2I = 136,

        L2F = 137,

        L2D = 138,

        F2I = 139,

        F2L = 140,

        F2D = 141,

        D2I = 142,

        D2L = 143,

        D2F = 144,

        I2B = 145,

        I2C = 146,

        I2S = 147,

        LCMP = 148,

        FCMPL = 149,

        FCMPG = 150,

        DCMPL = 151,

        DCMPG = 152,

        IFEQ = 153,

        IFNE = 154,

        IFLT = 155,

        IFGE = 156,

        IFGT = 157,

        IFLE = 158,

        IF_ICMPEQ = 159,

        IF_ICMPNE = 160,

        IF_ICMPLT = 161,

        IF_ICMPGE = 162,

        IF_ICMPGT = 163,

        IF_ICMPLE = 164,

        IF_ACMPEQ = 165,

        IF_ACMPNE = 166,

        GOTO = 167,

        JSR = 168,

        RET = 169,

        TABLESWITCH = 170,

        LOOKUPSWITCH = 171,

        IRETURN = 172,

        LRETURN = 173,

        FRETURN = 174,

        DRETURN = 175,

        ARETURN = 176,

        RETURN = 177,

        GETSTATIC = 178,

        PUTSTATIC = 179,

        GETFIELD = 180,

        PUTFIELD = 181,

        INVOKEVIRTUAL = 182,

        INVOKESPECIAL = 183,

        INVOKESTATIC = 184,

        INVOKEINTERFACE = 185,

        INVOKEDYNAMIC = 186,

        NEW = 187,

        NEWARRAY = 188,

        ANEWARRAY = 189,

        ARRAYLENGTH = 190,

        ATHROW = 191,

        CHECKCAST = 192,

        INSTANCEOF = 193,

        MONITORENTER = 194,

        MONITOREXIT = 195,

        WIDE = 196,

        MULTIANEWARRAY = 197,

        IFNULL = 198,

        IFNONNULL = 199,

        GOTO_W = 200,

        JSR_W = 201,

        BREAKPOINT = 202,

        LDC_QUICK = 203,

        LDC_W_QUICK = 204,

        LDC2_W_QUICK = 205,

        GETFIELD_QUICK = 206,

        PUTFIELD_QUICK = 207,

        GETFIELD2_QUICK = 208,

        PUTFIELD2_QUICK = 209,

        GETSTATIC_QUICK = 210,

        PUTSTATIC_QUICK = 211,

        GETSTATIC2_QUICK = 212,

        PUTSTATIC2_QUICK = 213,

        INVOKEVIRTUAL_QUICK = 214,

        INVOKENONVIRTUAL_QUICK = 215,

        INVOKESUPER_QUICK = 216,

        INVOKESTATIC_QUICK = 217,

        INVOKEINTERFACE_QUICK = 218,

        INVOKEVIRTUALOBJECT_QUICK = 219,

        NEW_QUICK = 221,

        ANEWARRAY_QUICK = 222,

        MULTIANEWARRAY_QUICK = 223,

        CHECKCAST_QUICK = 224,

        INSTANCEOF_QUICK = 225,

        INVOKEVIRTUAL_QUICK_W = 226,

        GETFIELD_QUICK_W = 227,

        PUTFIELD_QUICK_W = 228,

        IMPDEP1 = 254,

        IMPDEP2 = 255,
    };

    struct Instruction {
        OpCodeEnum opcode;
        u1 paramsCount{};
        std::unique_ptr<i4[]> params;
    };
}

#endif