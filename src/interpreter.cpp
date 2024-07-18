#include "interpreter.hpp"
#include <cmath>

#include "exception_helper.hpp"
#include "utils/byte_reader.hpp"
#include "utils/descriptor_parser.hpp"
#include "utils/class_utils.hpp"
#include "opcode.hpp"
#include "constant_info.hpp"
#include "frame.hpp"
#include "class.hpp"
#include "oop.hpp"
#include "class_loader.hpp"
#include "constant_pool.hpp"
#include "memory.hpp"
#include "invoke_dynamic.hpp"

namespace RexVM {

    namespace ByteHandler {
        void nop([[maybe_unused]] Frame &frame) {}

        void aconst_null(Frame &frame) {
            frame.pushRef(nullptr);
        }

        void iconst_m1(Frame &frame) {
            frame.pushI4(-1);
        }

        void iconst_0(Frame &frame) {
            frame.pushI4(0);
        }

        void iconst_1(Frame &frame) {
            frame.pushI4(1);
        }

        void iconst_2(Frame &frame) {
            frame.pushI4(2);
        }

        void iconst_3(Frame &frame) {
            frame.pushI4(3);
        }

        void iconst_4(Frame &frame) {
            frame.pushI4(4);
        }

        void iconst_5(Frame &frame) {
            frame.pushI4(5);
        }

        void lconst_0(Frame &frame) {
            frame.pushI8(0);
        }

        void lconst_1(Frame &frame) {
            frame.pushI8(1);
        }

        void fconst_0(Frame &frame) {
            frame.pushF4(0);
        }

        void fconst_1(Frame &frame) {
            frame.pushF4(1);
        }

        void fconst_2(Frame &frame) {
            frame.pushF4(2);
        }

        void dconst_0(Frame &frame) {
            frame.pushF8(0);
        }

        void dconst_1(Frame &frame) {
            frame.pushF8(1);
        }

        void bipush(Frame &frame) {
            frame.pushI4(frame.reader.readI1());
        }

        void sipush(Frame &frame) {
            frame.pushI4(frame.reader.readI2());
        }

        void ldc_(Frame &frame, u2 index) {
            const auto &constantPool = frame.constantPool;
            const auto valPtr = constantPool.at(index).get();
            const auto constantTagEnum = CAST_CONSTANT_TAG_ENUM(valPtr->tag);

            switch (constantTagEnum) {
                case ConstantTagEnum::CONSTANT_Integer:
                    frame.pushI4((CAST_CONSTANT_INTEGER_INFO(valPtr))->value);
                    break;

                case ConstantTagEnum::CONSTANT_Float:
                    frame.pushF4((CAST_CONSTANT_FLOAT_INFO(valPtr))->value);
                    break;
                
                case ConstantTagEnum::CONSTANT_Long:
                    frame.pushI8((CAST_CONSTANT_LONG_INFO(valPtr))->value);
                    break;

                case ConstantTagEnum::CONSTANT_Double:
                    frame.pushF8((CAST_CONSTANT_DOUBLE_INFO(valPtr))->value);
                    break;

                case ConstantTagEnum::CONSTANT_String: {
                    const auto stringConstInfo = CAST_CONSTANT_STRING_INFO(valPtr);
                    const auto strValue = getConstantStringFromPool(constantPool, stringConstInfo->index);
                    frame.pushRef(frame.vm.stringPool->getInternString(strValue));
                    break;
                }

                case ConstantTagEnum::CONSTANT_Class: {
                    const auto classConstInfo = CAST_CONSTANT_CLASS_INFO(valPtr);
                    const auto className = getConstantStringFromPool(constantPool, classConstInfo->index);
                    const auto value = frame.classLoader.getClass(className);
                    frame.pushRef(value->mirror.get());
                    break;
                }

                default:
                   panic(cformat("ldc error {}", valPtr->tag));
            }
        }

        void ldc(Frame &frame) {
            const auto index = frame.reader.readU1();
            ldc_(frame, index);
        }

        void ldc_w(Frame &frame) {
            const auto index = frame.reader.readU2();
            ldc_(frame, index);
        }

        void iload(Frame &frame) {
            const auto index = frame.reader.readU1();
            frame.pushLocal(index);
        }

        void lload(Frame &frame) {
            const auto index = frame.reader.readU1();
            frame.pushLocalWide(index);
        }

        void fload(Frame &frame) {
            const auto index = frame.reader.readU1();
            frame.pushLocal(index);
        }

        void dload(Frame &frame) {
            const auto index = frame.reader.readU1();
            frame.pushLocalWide(index);
        }

        void aload(Frame &frame) {
            const auto index = frame.reader.readU1();
            frame.pushLocal(index);
        }

        void iload_n(Frame &frame) {
            const auto index = frame.currentByteCode - CAST_U1(OpCodeEnum::ILOAD_0);
            frame.pushLocal(index);
        }

        void lload_n(Frame &frame) {
            const auto index = frame.currentByteCode - CAST_U1(OpCodeEnum::LLOAD_0);
            frame.pushLocalWide(index);
        }

        void fload_n(Frame &frame) {
            const auto index = frame.currentByteCode - CAST_U1(OpCodeEnum::FLOAD_0);
            frame.pushLocal(index);
        }

        void dload_n(Frame &frame) {
            const auto index = frame.currentByteCode - CAST_U1(OpCodeEnum::DLOAD_0);
            frame.pushLocalWide(index);
        }

        void aload_n(Frame &frame) {
            const auto index = frame.currentByteCode - CAST_U1(OpCodeEnum::ALOAD_0);
            frame.pushLocal(index);
        }

        void iaload(Frame &frame) {
            const auto index = frame.popI4();
            const auto array = CAST_INT_TYPE_ARRAY_OOP(frame.popRef());
            frame.pushI4(array->data[index]);
        }

        void laload(Frame &frame) {
            const auto index = frame.popI4();
            const auto array = CAST_LONG_TYPE_ARRAY_OOP(frame.popRef());
            frame.pushI8(array->data[index]);
        }

        void faload(Frame &frame) {
            const auto index = frame.popI4();
            const auto array = CAST_FLOAT_TYPE_ARRAY_OOP(frame.popRef());
            frame.pushF4(array->data[index]);
        }

        void daload(Frame &frame) {
            const auto index = frame.popI4();
            const auto array = CAST_DOUBLE_TYPE_ARRAY_OOP(frame.popRef());
            frame.pushF8(array->data[index]);
        }

        void aaload(Frame &frame) {
            const auto index = frame.popI4();
            const auto array = CAST_OBJ_ARRAY_OOP(frame.popRef());
            frame.pushRef(array->data[index]);
        }

        void baload(Frame &frame) {
            const auto index = frame.popI4();
            const auto array = CAST_BYTE_TYPE_ARRAY_OOP(frame.popRef());
            frame.pushI4(array->data[index]);
        }

        void caload(Frame &frame) {
            const auto index = frame.popI4();
            const auto array = CAST_CHAR_TYPE_ARRAY_OOP(frame.popRef());
            frame.pushI4(array->data[index]);
        }

        void saload(Frame &frame) {
            const auto index = frame.popI4();
            const auto array = CAST_SHORT_TYPE_ARRAY_OOP(frame.popRef());
            frame.pushI4(array->data[index]);
        }

        void istore(Frame &frame) {
            const auto index = frame.reader.readU1();
            frame.popLocal(index);
        }

        void lstore(Frame &frame) {
            const auto index = frame.reader.readU1();
            frame.popLocalWide(index);
        }

        void fstore(Frame &frame) {
            const auto index = frame.reader.readU1();
            frame.popLocal(index);
        }

        void dstore(Frame &frame) {
            const auto index = frame.reader.readU1();
            frame.popLocalWide(index);
        }

        void astore(Frame &frame) {
            const auto index = frame.reader.readU1();
            frame.popLocal(index);
        }

        void istorn_n(Frame &frame) {
            const auto index = frame.currentByteCode - CAST_U1(OpCodeEnum::ISTORE_0);
            frame.popLocal(index);
        }

        void lstorn_n(Frame &frame) {
            const auto index = frame.currentByteCode - CAST_U1(OpCodeEnum::LSTORE_0);
            frame.popLocalWide(index);
        }

        void fstorn_n(Frame &frame) {
            const auto index = frame.currentByteCode - CAST_U1(OpCodeEnum::FSTORE_0);
            frame.popLocal(index);
        }

        void dstorn_n(Frame &frame) {
            const auto index = frame.currentByteCode - CAST_U1(OpCodeEnum::DSTORE_0);
            frame.popLocalWide(index);
        }

        void astorn_n(Frame &frame) {
            const auto index = frame.currentByteCode - CAST_U1(OpCodeEnum::ASTORE_0);
            frame.popLocal(index);
        }

        void iastore(Frame &frame) {
            const auto val = frame.popI4();
            const auto index = frame.popI4();
            const auto array = CAST_INT_TYPE_ARRAY_OOP(frame.popRef());
            array->data[index] = val;
        }

        void lastore(Frame &frame) {
            const auto val = frame.popI8();
            const auto index = frame.popI4();
            const auto array = CAST_LONG_TYPE_ARRAY_OOP(frame.popRef());
            array->data[index] = val;
        }

        void fastore(Frame &frame) {
            const auto val = frame.popF4();
            const auto index = frame.popI4();
            const auto array = CAST_FLOAT_TYPE_ARRAY_OOP(frame.popRef());
            array->data[index] = val;
        }

        void dastore(Frame &frame) {
            const auto val = frame.popF8();
            const auto index = frame.popI4();
            const auto array = CAST_DOUBLE_TYPE_ARRAY_OOP(frame.popRef());
            array->data[index] = val;
        }

        void aastore(Frame &frame) {
            const auto val = frame.popRef();
            const auto index = frame.popI4();
            const auto array = CAST_OBJ_ARRAY_OOP(frame.popRef());
            array->data[index] = val;
        }

        void bastore(Frame &frame) {
            const auto val = frame.popI4();
            const auto index = frame.popI4();
            const auto array = CAST_BYTE_TYPE_ARRAY_OOP(frame.popRef());
            array->data[index] = val;
        }

        void castore(Frame &frame) {
            const auto val = frame.popI4();
            const auto index = frame.popI4();
            const auto array = CAST_CHAR_TYPE_ARRAY_OOP(frame.popRef());
            array->data[index] = val;
        }

        void sastore(Frame &frame) {
            const auto val = frame.popI4();
            const auto index = frame.popI4();
            const auto array = CAST_SHORT_TYPE_ARRAY_OOP(frame.popRef());
            array->data[index] = val;
        }

        void pop(Frame &frame) {
            frame.pop();
        }

        void pop2(Frame &frame) {
            frame.pop();
            frame.pop();
        }

        void dup(Frame &frame) {
            frame.operandStackContext.dup();
        }

        void dup_x1(Frame &frame) {
            frame.operandStackContext.dup_x1();
        }

        void dup_x2(Frame &frame) {
            frame.operandStackContext.dup_x2();
        }

        void dup2(Frame &frame) {
            frame.operandStackContext.dup2();
        }

        void dup2_x1(Frame &frame) {
            frame.operandStackContext.dup2_x1();
        }

        void dup2_x2(Frame &frame) {
            frame.operandStackContext.dup2_x2();
        }

        void swap(Frame &frame) {
            frame.operandStackContext.swapTop();
        }

        void iadd(Frame &frame) {
            const auto val2 = frame.popI4();   
            const auto val1 = frame.popI4();
            const auto result = CAST_I4(CAST_U4(val1) + CAST_U4(val2));
            frame.pushI4(result);
        }

        void ladd(Frame &frame) {
            const auto val2 = frame.popI8();   
            const auto val1 = frame.popI8();
            const auto result = CAST_I8(CAST_U8(val1) + CAST_U8(val2));
            frame.pushI8(result);
        }

        void fadd(Frame &frame) {
            const auto val2 = frame.popF4();   
            const auto val1 = frame.popF4();
            frame.pushF4(val1 + val2);
        }

        void dadd(Frame &frame) {
            const auto val2 = frame.popF8();   
            const auto val1 = frame.popF8();
            frame.pushF8(val1 + val2);
        }

        void isub(Frame &frame) {
            const auto val2 = frame.popI4();   
            const auto val1 = frame.popI4();
            frame.pushI4(val1 - val2);
        }

        void lsub(Frame &frame) {
            const auto val2 = frame.popI8();   
            const auto val1 = frame.popI8();
            frame.pushI8(val1 - val2);
        }

        void fsub(Frame &frame) {
            const auto val2 = frame.popF4();   
            const auto val1 = frame.popF4();
            frame.pushF4(val1 - val2);
        }

        void dsub(Frame &frame) {
            const auto val2 = frame.popF8();   
            const auto val1 = frame.popF8();
            frame.pushF8(val1 - val2);
        }

        void imul(Frame &frame) {
            const auto val2 = frame.popI4();   
            const auto val1 = frame.popI4();
            const auto result = CAST_I4(CAST_U4(val1) * CAST_U4(val2));
            frame.pushI4(result);
        }

        void lmul(Frame &frame) {
            const auto val2 = frame.popI8();   
            const auto val1 = frame.popI8();
            const auto result = CAST_I8(CAST_U8(val1) * CAST_U8(val2));
            frame.pushI8(result);
        }

        void fmul(Frame &frame) {
            const auto val2 = frame.popF4();   
            const auto val1 = frame.popF4();
            frame.pushF4(val1 * val2);
        }

        void dmul(Frame &frame) {
            const auto val2 = frame.popF8();   
            const auto val1 = frame.popF8();
            frame.pushF8(val1 * val2);
        }

        void idiv(Frame &frame) {
            const auto val2 = frame.popI4();   
            const auto val1 = frame.popI4();
            if (val2 == 0) {
                throwArithmeticExceptionDivByZero(frame);
                return;
            }
            frame.pushI4(val1 / val2);
        }

        void ldiv(Frame &frame) {
            const auto val2 = frame.popI8();   
            const auto val1 = frame.popI8();
            if (val2 == 0) {
                throwArithmeticExceptionDivByZero(frame);
                return;
            }
            frame.pushI8(val1 / val2);
        }

        void fdiv(Frame &frame) {
            const auto val2 = frame.popF4();   
            const auto val1 = frame.popF4();
            if (val2 == 0) {
                throwArithmeticExceptionDivByZero(frame);
                return;
            }
            frame.pushF4(val1 / val2);
        }

        void ddiv(Frame &frame) {
            const auto val2 = frame.popF8();   
            const auto val1 = frame.popF8();
            if (val2 == 0) {
                throwArithmeticExceptionDivByZero(frame);
                return;
            }
            frame.pushF8(val1 / val2);
        }

        void irem(Frame &frame) {
            const auto val2 = frame.popI4();   
            const auto val1 = frame.popI4();
            if (val2 == 0) {
                throwArithmeticExceptionDivByZero(frame);
                return;
            }
            frame.pushI4(val1 % val2);
        }

        void lrem(Frame &frame) {
            const auto val2 = frame.popI8();   
            const auto val1 = frame.popI8();
            if (val2 == 0) {
                throwArithmeticExceptionDivByZero(frame);
                return;
            }
            frame.pushI8(val1 % val2);
        }

         void frem(Frame &frame) {
            const auto val2 = frame.popF4();   
            const auto val1 = frame.popF4();
            if (val2 == 0) {
                throwArithmeticExceptionDivByZero(frame);
                return;
            }
            const auto result = std::fmod(val1, val2);
            frame.pushF4(result);
        }

        void drem(Frame &frame) {
            const auto val2 = frame.popF8();   
            const auto val1 = frame.popF8();
            if (val2 == 0) {
                throwArithmeticExceptionDivByZero(frame);
                return;
            }
            const auto result = std::fmod(val1, val2);
            frame.pushF8(result);
        }

        void ineg(Frame &frame) {
            frame.pushI4(-frame.popI4());
        }

        void lneg(Frame &frame) {
            frame.pushI8(-frame.popI8());
        }

        void fneg(Frame &frame) {
            frame.pushF4(-frame.popF4());
        }

        void dneg(Frame &frame) {
            frame.pushF8(-frame.popF8());
        }

        void ishl(Frame &frame) {
            const auto val2 = frame.popI4();   
            const auto val1 = frame.popI4();
            const auto s = (CAST_U4(val2)) & 0x1F;
            frame.pushI4(val1 << s);
        }

        void lshl(Frame &frame) {
            const auto val2 = frame.popI4();   
            const auto val1 = frame.popI8();
            const auto s = (CAST_U4(val2)) & 0x3F;
            frame.pushI8(val1 << s);
        }

        void ishr(Frame &frame) {
            const auto val2 = frame.popI4();   
            const auto val1 = frame.popI4();
            const auto s = (CAST_U4(val2)) & 0x1F;
            frame.pushI4(val1 >> s);
        }

        void lshr(Frame &frame) {
            const auto val2 = frame.popI4();   
            const auto val1 = frame.popI8();
            const auto s = (CAST_U4(val2)) & 0x3F;
            frame.pushI8(val1 >> s);
        }

        void iushr(Frame &frame) {
            const auto val2 = frame.popI4();   
            const auto val1 = frame.popI4();
            const auto s = (CAST_U4(val2)) & 0x1F;
            frame.pushI4(CAST_U4(val1) >> s);
        }

        void lushr(Frame &frame) {
            const auto val2 = frame.popI4();   
            const auto val1 = frame.popI8();
            const auto s = (CAST_U4(val2)) & 0x3F;
            frame.pushI8(CAST_U8(val1) >> s);
        }

        void iand(Frame &frame) {
            const auto val2 = frame.popI4();   
            const auto val1 = frame.popI4();
            frame.pushI4(val1 & val2);
        }

        void land(Frame &frame) {
            const auto val2 = frame.popI8();   
            const auto val1 = frame.popI8();
            frame.pushI8(val1 & val2);
        }

        void ior(Frame &frame) {
            const auto val2 = frame.popI4();   
            const auto val1 = frame.popI4();
            frame.pushI4(val1 | val2);
        }

        void lor(Frame &frame) {
            const auto val2 = frame.popI8();   
            const auto val1 = frame.popI8();
            frame.pushI8(val1 | val2);
        }

        void ixor(Frame &frame) {
            const auto val2 = frame.popI4();   
            const auto val1 = frame.popI4();
            frame.pushI4(val1 ^ val2); 
        }

        void lxor(Frame &frame) {
            const auto val2 = frame.popI8();   
            const auto val1 = frame.popI8();
            frame.pushI8(val1 ^ val2);
        }

        //Mark

        void iinc(Frame &frame) {
            const auto index = frame.reader.readU1();
            const auto value = frame.reader.readI1();
            frame.setLocalI4(index, frame.getLocalI4(index) + value);
        }

        void i2l(Frame &frame) {
            const auto val = frame.popI4();
            frame.pushI8(CAST_I8(val));
        }
        
        void i2f(Frame &frame) {
            const auto val = frame.popI4();
            frame.pushF4(CAST_F4(val));
        }

        void i2d(Frame &frame) {
            const auto val = frame.popI4();
            frame.pushF8(CAST_F8(val));
        }

        void l2i(Frame &frame) {
            const auto val = frame.popI8();
            frame.pushI4(CAST_I4(val));
        }

        void l2f(Frame &frame) {
            const auto val = frame.popI8();
            frame.pushF4(CAST_F4(val));
        }

        void l2d(Frame &frame) {
            const auto val = frame.popI8();
            frame.pushF8(CAST_F8(val));
        }

        void f2i(Frame &frame) {
            const auto val = frame.popF4();
            frame.pushI4(CAST_I4(val));
        }

        void f2l(Frame &frame) {
            const auto val = frame.popF4();
            frame.pushI8(CAST_I8(val));
        }

        void f2d(Frame &frame) {
            const auto val = frame.popF4();
            frame.pushF8(CAST_F8(val));
        }

        void d2i(Frame &frame) {
            const auto val = frame.popF8();
            frame.pushI4(CAST_I4(val));
        }

        void d2l(Frame &frame) {
            const auto val = frame.popF8();
            frame.pushI8(CAST_I8(val));
        }

        void d2f(Frame &frame) {
            const auto val = frame.popF8();
            frame.pushF4(CAST_F4(val));
        }

        void i2b(Frame &frame) {
            const auto val = frame.popI4();
            frame.pushI4(CAST_I1(val));
        }

        void i2c(Frame &frame) {
            const auto val = frame.popI4();
            frame.pushI4(CAST_U2(val));
        }

        void i2s(Frame &frame) {
            const auto val = frame.popI4();
            frame.pushI4(CAST_I2(val));
        }

        void lcmp(Frame &frame) {
            const auto val2 = frame.popI8();
            const auto val1 = frame.popI8();
            if (val1 > val2) {
                frame.pushI4(1);
            } else if (val1 == val2) {
                frame.pushI4(0);
            } else {
                frame.pushI4(-1);
            }
        }

        void fcmp_(Frame &frame, bool gCheck) {
            const auto val2 = frame.popF4();
            const auto val1 = frame.popF4();
            if (val1 > val2) {
                frame.pushI4(1);
            } else if (val1 == val2) {
                frame.pushI4(0);
            } else if (val1 < val2) {
                frame.pushI4(-1);
            } else if (gCheck) {
                frame.pushI4(1);
            } else {
                frame.pushI4(-1);
            }
        }

        void fcmpl(Frame &frame) {
            fcmp_(frame, false);
        }

        void fcmpg(Frame &frame) {
            fcmp_(frame, true);
        }

        void dcmp_(Frame &frame, bool gCheck) {
            const auto val2 = frame.popF8();
            const auto val1 = frame.popF8();
            if (val1 > val2) {
                frame.pushI4(1);
            } else if (val1 == val2) {
                frame.pushI4(0);
            } else if (val1 < val2) {
                frame.pushI4(-1);
            } else if (gCheck) {
                frame.pushI4(1);
            } else {
                frame.pushI4(-1);
            }
        }

        void dcmpl(Frame &frame) {
            dcmp_(frame, false);
        }

        void dcmpg(Frame &frame) {
            dcmp_(frame, true);
        }

        void ifeq(Frame &frame) {
            const auto offset = frame.reader.readI2();
            const auto val = frame.popI4();
            if (val == 0) {
                frame.reader.relativeOffset(offset);
            }
        }

        void ifne(Frame &frame) {
            const auto offset = frame.reader.readI2();
            const auto val = frame.popI4();
            if (val != 0) {
                frame.reader.relativeOffset(offset);
            }
        }

        void iflt(Frame &frame) {
            const auto offset = frame.reader.readI2();
            const auto val = frame.popI4();
            if (val < 0) {
                frame.reader.relativeOffset(offset);
            }
        }

        void ifge(Frame &frame) {
            const auto offset = frame.reader.readI2();
            const auto val = frame.popI4();
            if (val >= 0) {
                frame.reader.relativeOffset(offset);
            }
        }

        void ifgt(Frame &frame) {
            const auto offset = frame.reader.readI2();
            const auto val = frame.popI4();
            if (val > 0) {
                frame.reader.relativeOffset(offset);
            }
        }

        void ifle(Frame &frame) {
            const auto offset = frame.reader.readI2();
            const auto val = frame.popI4();
            if (val <= 0) {
                frame.reader.relativeOffset(offset);
            }
        }

        void if_icmpeq(Frame &frame) {
            const auto offset = frame.reader.readI2();
            const auto val2 = frame.popI4();
            const auto val1 = frame.popI4();
            if (val1 == val2) {
                frame.reader.relativeOffset(offset);
            }
        }

        void if_icmpne(Frame &frame) {
            const auto offset = frame.reader.readI2();
            const auto val2 = frame.popI4();
            const auto val1 = frame.popI4();
            if (val1 != val2) {
                frame.reader.relativeOffset(offset);
            }
        }

        void if_icmplt(Frame &frame) {
            const auto offset = frame.reader.readI2();
            const auto val2 = frame.popI4();
            const auto val1 = frame.popI4();
            if (val1 < val2) {
                frame.reader.relativeOffset(offset);
            }
        }

        void if_icmpge(Frame &frame) {
            const auto offset = frame.reader.readI2();
            const auto val2 = frame.popI4();
            const auto val1 = frame.popI4();
            if (val1 >= val2) {
                frame.reader.relativeOffset(offset);
            }
        }

        void if_icmpgt(Frame &frame) {
            const auto offset = frame.reader.readI2();
            const auto val2 = frame.popI4();
            const auto val1 = frame.popI4();
            if (val1 > val2) {
                frame.reader.relativeOffset(offset);
            }
        }

        void if_icmple(Frame &frame) {
            const auto offset = frame.reader.readI2();
            const auto val2 = frame.popI4();
            const auto val1 = frame.popI4();
            if (val1 <= val2) {
                frame.reader.relativeOffset(offset);
            }
        }

        void if_acmpeq(Frame &frame) {
            const auto offset = frame.reader.readI2();
            const auto val2 = frame.popRef();
            const auto val1 = frame.popRef();
            if (val1 == val2) {
                frame.reader.relativeOffset(offset);
            }
        }

        void if_acmpne(Frame &frame) {
            const auto offset = frame.reader.readI2();
            const auto val2 = frame.popRef();
            const auto val1 = frame.popRef();
            if (val1 != val2) {
                frame.reader.relativeOffset(offset);
            }
        }

        void goto_(Frame &frame) {
            const auto offset = frame.reader.readI2();
            frame.reader.relativeOffset(offset);
        }

        void jsr(Frame &frame) {
            panic("jsr not implement!");
        }

        void ret(Frame &frame) {
            panic("ret not implement!");
        }

        void tableswitch(Frame &frame) {
            while (frame.nextPc() % 4 != 0) {
                frame.reader.readU1();
	        }
            const auto defaultOffset = frame.reader.readI4();
	        const auto low = frame.reader.readI4();
	        const auto high = frame.reader.readI4();
	        const auto jumpOffsetsCount = high - low + 1;
            auto jumpOffsets = std::make_unique<i4[]>(jumpOffsetsCount);
            for (i4 i = 0; i < jumpOffsetsCount; ++i) {
                jumpOffsets[i] = frame.reader.readI4();
            }

            const auto index = frame.popI4();
            i4 offset = 0;
	        if (index >= low && index <= high) {
		        offset = CAST_I4(jumpOffsets[index-low]);
	        } else {
		        offset = CAST_I4(defaultOffset);
	        }

            frame.reader.relativeOffset(offset);
        }

        void lookupswitch(Frame &frame) {
            while (frame.nextPc() % 4 != 0) {
                frame.reader.readU1();
	        }
            const auto defaultOffset = frame.reader.readI4();
	        const auto npairs = frame.reader.readI4();
            const auto count = npairs * 2;
            auto matchOffsets = std::make_unique<i4[]>(count);

            for (i4 i = 0; i < count; ++i) {
                matchOffsets[i] = frame.reader.readI4();
            }

            const auto key = frame.popI4();
            for (i4 i = 0; i < count; i += 2) {
                if (matchOffsets[i] == key) {
			        const auto offset = matchOffsets[i+1];
                    frame.reader.relativeOffset(offset);
			        return;
		        }
            }

            frame.reader.relativeOffset(defaultOffset);
        }

        void ireturn(Frame &frame) {
            const auto val = frame.popI4();
            frame.returnI4(val);
        }

        void lreturn(Frame &frame) {
            const auto val = frame.popI8();
            frame.returnI8(val);
        }

        void freturn(Frame &frame) {
            const auto val = frame.popF4();
            frame.returnF4(val);
        }

        void dreturn(Frame &frame) {
            const auto val = frame.popF8();
            frame.returnF8(val);
        }

        void areturn(Frame &frame) {
            const auto val = frame.popRef();
            frame.returnRef(val);
        }

        void vreturn(Frame &frame) {
            frame.returnVoid();
        }

        void getstatic(Frame &frame) {
            const auto index = frame.reader.readU2();
            const auto fieldRef = frame.klass.getRefField(index, true);
            auto &fieldClass = fieldRef->klass;
            fieldClass.clinit(frame);
            const auto value = fieldClass.getFieldValue(fieldRef->slotId);
            const auto type = fieldRef->getFieldSlotType();
            frame.push(value, type);
            if (fieldRef->isWideType()) {
                frame.push(Slot(0), type);
            }
        }

        void putstatic(Frame &frame) {
            const auto index = frame.reader.readU2();
            const auto fieldRef = frame.klass.getRefField(index, true);
            auto &fieldClass = fieldRef->klass;
            fieldClass.clinit(frame);
            if (fieldRef->isWideType()) {
                frame.pop();
            }

            const auto value = frame.pop();
            fieldClass.setFieldValue(fieldRef->slotId, value);
        }

        void getfield(Frame &frame) {
            const auto index = frame.reader.readU2();
            const auto fieldRef = frame.klass.getRefField(index, false);
            const auto instance = CAST_INSTANCE_OOP(frame.popRef());
            const auto value = instance->getFieldValue(fieldRef->slotId);
            const auto type = fieldRef->getFieldSlotType();
            frame.push(value, type);
            if (fieldRef->isWideType()) {
                frame.push(Slot(0), type);
            }
        }

        void putfield(Frame &frame) {
            const auto index = frame.reader.readU2();
            const auto fieldRef = frame.klass.getRefField(index, false);
            if (fieldRef->isWideType()) {
                frame.pop();
            }
            const auto value = frame.pop();
            const auto instance = CAST_INSTANCE_OOP(frame.popRef());

            instance->setFieldValue(fieldRef->slotId, value);
        }
        
        void invokeMethodHandle(
            Frame &frame, 
            const cstring &className, 
            const cstring &methodName, 
            const cstring &methodDescriptor
        ) {
            auto &classLoader = *frame.getCurrentClassLoader();
            const auto invokeMethod = 
                    classLoader.getInstanceClass("java/lang/invoke/MethodHandle")
                        ->getMethod(methodName, METHOD_HANDLE_INVOKE_ORIGIN_DESCRITPR, false);
            const auto [paramType, _] = parseMethodDescriptor(methodDescriptor); 
            //1是因为第一个参数为MethodHandle Object
            size_t popLength = 1;
            for (const auto &paramClassName : paramType) {
                if (isWideClassName(paramClassName)) {
                    popLength += 2;
                } else {
                    popLength += 1;
                }
            }
            frame.runMethodInner(*invokeMethod, popLength);
        }

        void invokevirtual(Frame &frame) {
            const auto &constantPool = frame.constantPool;
            const auto index = frame.reader.readU2();
            auto [className, methodName, methodDescriptor] = 
                    getConstantStringFromPoolByClassNameType(constantPool, index);
            if (isMethodHandleInvoke(className, methodName)) {
                invokeMethodHandle(frame, className, methodName, methodDescriptor);
                return;
            }

            const auto invokeMethod = frame.klass.getRefMethod(index, false);
            const auto instance = frame.getStackOffset(invokeMethod->paramSlotSize - 1).refVal;
            const auto instanceClass = CAST_INSTANCE_CLASS(instance->klass);
            for (auto k = instanceClass; k != nullptr; k = k->superClass) {
                const auto realInvokeMethod =
                        k->getMethod(invokeMethod->name, invokeMethod->descriptor, invokeMethod->isStatic());
                if (realInvokeMethod != nullptr) {
                    frame.runMethodInner(*realInvokeMethod);
                    return;
                }
            }
            panic("invoke failed");
        }


        /*
        原版 invokevirtual
        void invokevirtual(Frame &frame) {
            const auto index = frame.reader.readU2();
            const auto invokeMethod = frame.klass.getRefMethod(index, false);
            const auto instance = frame.getStackOffset(invokeMethod->paramSlotSize - 1).refVal;
            const auto instanceClass = CAST_INSTANCE_CLASS(instance->klass);
            for (auto k = instanceClass; k != nullptr; k = k->superClass) {
                const auto realInvokeMethod =
                        k->getMethod(invokeMethod->name, invokeMethod->descriptor, invokeMethod->isStatic());
                if (realInvokeMethod != nullptr) {
                    frame.runMethodInner(*realInvokeMethod);
                    return;
                }
            }
            panic("invoke failed");
        }
        */

        void invokespecial(Frame &frame) {
            const auto index = frame.reader.readU2();
            const auto invokeMethod = frame.klass.getRefMethod(index, false);
            frame.runMethodInner(*invokeMethod);
        }

        void invokestatic(Frame &frame) {
            const auto index = frame.reader.readU2();
            const auto invokeMethod = frame.klass.getRefMethod(index, true);
            invokeMethod->klass.clinit(frame);
            frame.runMethodInner(*invokeMethod);
        }

        void invokeinterface(Frame &frame) {
            const auto index = frame.reader.readU2();
            frame.reader.readU2(); //ignore

            const auto invokeMethod = frame.klass.getRefMethod(index, false);
            const auto instance = frame.getStackOffset(invokeMethod->paramSlotSize - 1).refVal;
            const auto instanceClass = CAST_INSTANCE_CLASS(instance->klass);
            const auto realInvokeMethod = instanceClass->getMethod(invokeMethod->name, invokeMethod->descriptor, invokeMethod->isStatic());
            if (realInvokeMethod != nullptr) {
                frame.runMethodInner(*realInvokeMethod);
            } else if (!invokeMethod->isAbstract()) {
                frame.runMethodInner(*invokeMethod);
            } else {
                //TODO
                panic("can't find method");
            }
        }

        void invokedynamic(Frame &frame) {
            const auto index = frame.reader.readU2();
            frame.reader.readU2(); //ignore zero

            invokeDynamic(frame, index);

        }

        void new_(Frame &frame) {
            const auto index = frame.reader.readU2();
            const auto &constantPool = frame.constantPool;
            const auto className = getConstantStringFromPoolByIndexInfo(constantPool, index);
            const auto instanceClass = frame.classLoader.getInstanceClass(className);
            instanceClass->clinit(frame);
            const auto &oopManager = frame.vm.oopManager;

            const auto specialType = instanceClass->specialInstanceClass;
            switch (specialType) {
                case SpecialInstanceClass::NONE:
                    frame.pushRef(oopManager->newInstance(instanceClass));
                    break;
                
                case SpecialInstanceClass::THREAD_CLASS:
                    frame.pushRef(oopManager->newVMThread(instanceClass));
                    break;
                
                case SpecialInstanceClass::CLASS_LOADER_CLASS:
                    panic("not implement");
                    break;
            }
        }

        void newarray(Frame &frame) {
            const auto &oopManager = frame.vm.oopManager;
            const auto type = static_cast<BasicType>(frame.reader.readU1());
            const auto length = frame.popI4();
            const auto oop = oopManager->newTypeArrayOop(type, length);
            frame.pushRef(oop);
        }

        void anewarray(Frame &frame) {
            const auto &oopManager = frame.vm.oopManager;
            const auto classIndex = frame.reader.readU2();
            const auto length = frame.popI4();
            const auto &constantPool = frame.constantPool;
            const auto className = getConstantStringFromPoolByIndexInfo(constantPool, classIndex);
            const auto array = frame.classLoader.getObjectArrayClass(className);
            frame.pushRef(oopManager->newObjArrayOop(array, length));
        }

        void arraylength(Frame &frame) {
            const auto array = CAST_ARRAY_OOP(frame.popRef());
            frame.pushI4(array->dataLength);
        }

        void athrow(Frame &frame) {
            const auto ex = frame.popRef();
            const auto exOop = CAST_INSTANCE_OOP(ex);
            frame.throwException(exOop);
        }

        void checkcast(Frame &frame) {
            const auto index = frame.reader.readU2();
            const auto ref = frame.pop().refVal;
            frame.pushRef(ref);
            if (ref == nullptr) {
                return;
            }
            const auto &constantPool = frame.constantPool;
            const auto className = getConstantStringFromPoolByIndexInfo(constantPool, index);
            const auto checkClass = frame.classLoader.getClass(className);
            if (!ref->isInstanceOf(checkClass)) {
                //TODO frameThrow
                panic("checkcast error");
            }
        }

        void instanceof(Frame &frame) {
            const auto index = frame.reader.readU2();
            const auto ref = frame.pop().refVal;
            if (ref == nullptr) {
                frame.pushI4(0);
                return;
            }
            const auto &constantPool = frame.constantPool;
            const auto className = getConstantStringFromPoolByIndexInfo(constantPool, index);
            const auto checkClass = frame.classLoader.getClass(className);
            if (ref->isInstanceOf(checkClass)) {
                frame.pushI4(1);
            } else {
                frame.pushI4(0);
            }
        }

        void monitorenter(Frame &frame) {
            const auto oop = frame.popRef();
            oop->monitorMtx.lock();
        }

        void monitorexit(Frame &frame) {
            const auto oop = frame.popRef();
            oop->monitorMtx.unlock();
        }

        void wide(Frame &frame) {
            const auto opCode = static_cast<OpCodeEnum>(frame.reader.readU1());
            const auto index = frame.reader.readU2();

            switch (opCode) {
                case OpCodeEnum::ILOAD:
                case OpCodeEnum::FLOAD:
                case OpCodeEnum::ALOAD:
                    frame.pushLocal(index);
                break;

                case OpCodeEnum::LLOAD:
                case OpCodeEnum::DLOAD:
                    frame.pushLocalWide(index);
                break;
                

                case OpCodeEnum::ISTORE:
                case OpCodeEnum::FSTORE:
                case OpCodeEnum::ASTORE:
                    frame.popLocal(index);
                break;

                case OpCodeEnum::LSTORE:
                case OpCodeEnum::DSTORE:
                    frame.popLocalWide(index);
                break;

                case OpCodeEnum::IINC: {
                    const auto value = frame.reader.readI2();
                    frame.setLocalI4(index, frame.getLocalI4(index) + value);
                    break;
                }

                case OpCodeEnum::RET:
                    panic("ret not implement!");
                break;

                default:
                    panic("wide error");
            }
        }

        ref multiArrayHelper(OopManager &oopManager, ClassLoader &classLoader, std::unique_ptr<i4[]> &dimLength, i4 dimCount, const cstring& name, i4 currentDim) {
            const auto arrayLength = dimLength[currentDim];
            const auto currentArrayClass = classLoader.getArrayClass(name);
            ArrayOop *arrayOop;
            if (currentArrayClass->type == ClassTypeEnum::TypeArrayClass) {
                const auto typeArrayClass = CAST_TYPE_ARRAY_CLASS(currentArrayClass);
                arrayOop = oopManager.newTypeArrayOop(typeArrayClass->elementType, arrayLength);
            } else {
                const auto objArrayClass = CAST_OBJ_ARRAY_CLASS(currentArrayClass);
                arrayOop = oopManager.newObjArrayOop(objArrayClass, arrayLength);
            }

            if (currentDim == dimCount - 1) {
                return arrayOop;
            }

            auto objArrayOop = CAST_OBJ_ARRAY_OOP(arrayOop);
            if (currentDim < dimCount - 1) {
                const auto childName = name.substr(1);
                for (auto i = 0; i < arrayLength; ++i) {
                    objArrayOop->data[i] = multiArrayHelper(oopManager, classLoader, dimLength, dimCount, childName, currentDim + 1);
                }
            }
            return objArrayOop;
        }

        void multianewarray(Frame &frame) {
            const auto &oopManager = frame.vm.oopManager;
            auto &classLoader = frame.classLoader;
            const auto index = frame.reader.readU2();
            const auto dimension = frame.reader.readU1();
            const auto &constantPool = frame.constantPool;
            const auto className = getConstantStringFromPoolByIndexInfo(constantPool, index);
            auto dimLength = std::make_unique<i4[]>(dimension);
            for (i4 i = dimension - 1; i >= 0; --i) {
                dimLength[i] = frame.popI4();
            }
            
            const auto multiArray = multiArrayHelper(*oopManager, classLoader, dimLength, dimension, className, 0);
            frame.pushRef(multiArray);
        }

        void ifnull(Frame &frame) {
            const auto offset = frame.reader.readI2();
            const auto val = frame.popRef();

            if (val == nullptr) {
                frame.reader.relativeOffset(offset);
            }
        }

        void ifnonnull(Frame &frame) {
            const auto offset = frame.reader.readI2();
            const auto val = frame.popRef();

            if (val != nullptr) {
                frame.reader.relativeOffset(offset);
            }
        }

        void goto_w(Frame &frame) {
            const auto offset = frame.reader.readI4();
            frame.reader.relativeOffset(offset);
        }
        
    }

    std::array<MethodHandler, 256> OpCodeHandlers{
        ByteHandler::nop, // = 0:  nop
        ByteHandler::aconst_null, // = 1:  aconst_null
        ByteHandler::iconst_m1, // = 2:  iconst_m1
        ByteHandler::iconst_0, // = 3:  iconst_0
        ByteHandler::iconst_1, // = 4:  iconst_1
        ByteHandler::iconst_2, // = 5:  iconst_2
        ByteHandler::iconst_3, // = 6:  iconst_3
        ByteHandler::iconst_4, // = 7:  iconst_4
        ByteHandler::iconst_5, // = 8:  iconst_5
        ByteHandler::lconst_0, // = 9:  lconst_0
        ByteHandler::lconst_1, // = 10:  lconst_1
        ByteHandler::fconst_0, // = 11:  fconst_0
        ByteHandler::fconst_1, // = 12:  fconst_1
        ByteHandler::fconst_2, // = 13:  fconst_2
        ByteHandler::dconst_0, // = 14:  dconst_0
        ByteHandler::dconst_1, // = 15:  dconst_1
        ByteHandler::bipush, // = 16:  bipush
        ByteHandler::sipush, // = 17:  sipush
        ByteHandler::ldc, // = 18:  ldc
        ByteHandler::ldc_w, // = 19:  ldc_w
        ByteHandler::ldc_w, // = 20:  ldc2_w
        ByteHandler::iload, // = 21:  iload
        ByteHandler::lload, // = 22:  lload
        ByteHandler::fload, // = 23:  fload
        ByteHandler::dload, // = 24:  dload
        ByteHandler::aload, // = 25:  aload
        ByteHandler::iload_n, // = 26:  iload_0
        ByteHandler::iload_n, // = 27:  iload_1
        ByteHandler::iload_n, // = 28:  iload_2
        ByteHandler::iload_n, // = 29:  iload_3
        ByteHandler::lload_n, // = 30:  lload_0
        ByteHandler::lload_n, // = 31:  lload_1
        ByteHandler::lload_n, // = 32:  lload_2
        ByteHandler::lload_n, // = 33:  lload_3
        ByteHandler::fload_n, // = 34:  fload_0
        ByteHandler::fload_n, // = 35:  fload_1
        ByteHandler::fload_n, // = 36:  fload_2
        ByteHandler::fload_n, // = 37:  fload_3
        ByteHandler::dload_n, // = 38:  dload_0
        ByteHandler::dload_n, // = 39:  dload_1
        ByteHandler::dload_n, // = 40:  dload_2
        ByteHandler::dload_n, // = 41:  dload_3
        ByteHandler::aload_n, // = 42:  aload_0
        ByteHandler::aload_n, // = 43:  aload_1
        ByteHandler::aload_n, // = 44:  aload_2
        ByteHandler::aload_n, // = 45:  aload_3
        ByteHandler::iaload, // = 46:  iaload
        ByteHandler::laload, // = 47:  laload
        ByteHandler::faload, // = 48:  faload
        ByteHandler::daload, // = 49:  daload
        ByteHandler::aaload, // = 50:  aaload
        ByteHandler::baload, // = 51:  baload
        ByteHandler::caload, // = 52:  caload
        ByteHandler::saload, // = 53:  saload
        ByteHandler::istore, // = 54:  istore
        ByteHandler::lstore, // = 55:  lstore
        ByteHandler::fstore, // = 56:  fstore
        ByteHandler::dstore, // = 57:  dstore
        ByteHandler::astore, // = 58:  astore
        ByteHandler::istorn_n, // = 59:  istore_0
        ByteHandler::istorn_n, // = 60:  istore_1
        ByteHandler::istorn_n, // = 61:  istore_2
        ByteHandler::istorn_n, // = 62:  istore_3
        ByteHandler::lstorn_n, // = 63:  lstore_0
        ByteHandler::lstorn_n, // = 64:  lstore_1
        ByteHandler::lstorn_n, // = 65:  lstore_2
        ByteHandler::lstorn_n, // = 66:  lstore_3
        ByteHandler::fstorn_n, // = 67:  fstore_0
        ByteHandler::fstorn_n, // = 68:  fstore_1
        ByteHandler::fstorn_n, // = 69:  fstore_2
        ByteHandler::fstorn_n, // = 70:  fstore_3
        ByteHandler::dstorn_n, // = 71:  dstore_0
        ByteHandler::dstorn_n, // = 72:  dstore_1
        ByteHandler::dstorn_n, // = 73:  dstore_2
        ByteHandler::dstorn_n, // = 74:  dstore_3
        ByteHandler::astorn_n, // = 75:  astore_0
        ByteHandler::astorn_n, // = 76:  astore_1
        ByteHandler::astorn_n, // = 77:  astore_2
        ByteHandler::astorn_n, // = 78:  astore_3
        ByteHandler::iastore, // = 79:  iastore
        ByteHandler::lastore, // = 80:  lastore
        ByteHandler::fastore, // = 81:  fastore
        ByteHandler::dastore, // = 82:  dastore
        ByteHandler::aastore, // = 83:  aastore
        ByteHandler::bastore, // = 84:  bastore
        ByteHandler::castore, // = 85:  castore
        ByteHandler::sastore, // = 86:  sastore
        ByteHandler::pop, // = 87:  pop
        ByteHandler::pop2, // = 88:  pop2
        ByteHandler::dup, // = 89:  dup
        ByteHandler::dup_x1, // = 90:  dup_x1
        ByteHandler::dup_x2, // = 91:  dup_x2
        ByteHandler::dup2, // = 92:  dup2
        ByteHandler::dup2_x1, // = 93:  dup2_x1
        ByteHandler::dup2_x2, // = 94:  dup2_x2
        ByteHandler::swap, // = 95:  swap
        ByteHandler::iadd, // = 96:  iadd
        ByteHandler::ladd, // = 97:  ladd
        ByteHandler::fadd, // = 98:  fadd
        ByteHandler::dadd, // = 99:  dadd
        ByteHandler::isub, // = 100:  isub
        ByteHandler::lsub, // = 101:  lsub
        ByteHandler::fsub, // = 102:  fsub
        ByteHandler::dsub, // = 103:  dsub
        ByteHandler::imul, // = 104:  imul
        ByteHandler::lmul, // = 105:  lmul
        ByteHandler::fmul, // = 106:  fmul
        ByteHandler::dmul, // = 107:  dmul
        ByteHandler::idiv, // = 108:  idiv
        ByteHandler::ldiv, // = 109:  ldiv
        ByteHandler::fdiv, // = 110:  fdiv
        ByteHandler::ddiv, // = 111:  ddiv
        ByteHandler::irem, // = 112:  irem
        ByteHandler::lrem, // = 113:  lrem
        ByteHandler::frem, // = 114:  frem
        ByteHandler::drem, // = 115:  drem
        ByteHandler::ineg, // = 116:  ineg
        ByteHandler::lneg, // = 117:  lneg
        ByteHandler::fneg, // = 118:  fneg
        ByteHandler::dneg, // = 119:  dneg
        ByteHandler::ishl, // = 120:  ishl
        ByteHandler::lshl, // = 121:  lshl
        ByteHandler::ishr, // = 122:  ishr
        ByteHandler::lshr, // = 123:  lshr
        ByteHandler::iushr, // = 124:  iushr
        ByteHandler::lushr, // = 125:  lushr
        ByteHandler::iand, // = 126:  iand
        ByteHandler::land, // = 127:  land
        ByteHandler::ior, // = 128:  ior
        ByteHandler::lor, // = 129:  lor
        ByteHandler::ixor, // = 130:  ixor
        ByteHandler::lxor, // = 131:  lxor
        ByteHandler::iinc, // = 132:  iinc
        ByteHandler::i2l, // = 133:  i2l
        ByteHandler::i2f, // = 134:  i2f
        ByteHandler::i2d, // = 135:  i2d
        ByteHandler::l2i, // = 136:  l2i
        ByteHandler::l2f, // = 137:  l2f
        ByteHandler::l2d, // = 138:  l2d
        ByteHandler::f2i, // = 139:  f2i
        ByteHandler::f2l, // = 140:  f2l
        ByteHandler::f2d, // = 141:  f2d
        ByteHandler::d2i, // = 142:  d2i
        ByteHandler::d2l, // = 143:  d2l
        ByteHandler::d2f, // = 144:  d2f
        ByteHandler::i2b, // = 145:  i2b
        ByteHandler::i2c, // = 146:  i2c
        ByteHandler::i2s, // = 147:  i2s
        ByteHandler::lcmp, // = 148:  lcmp
        ByteHandler::fcmpl, // = 149:  fcmpl
        ByteHandler::fcmpg, // = 150:  fcmpg
        ByteHandler::dcmpl, // = 151:  dcmpl
        ByteHandler::dcmpg, // = 152:  dcmpg
        ByteHandler::ifeq, // = 153:  ifeq
        ByteHandler::ifne, // = 154:  ifne
        ByteHandler::iflt, // = 155:  iflt
        ByteHandler::ifge, // = 156:  ifge
        ByteHandler::ifgt, // = 157:  ifgt
        ByteHandler::ifle, // = 158:  ifle
        ByteHandler::if_icmpeq, // = 159:  if_icmpeq
        ByteHandler::if_icmpne, // = 160:  if_icmpne
        ByteHandler::if_icmplt, // = 161:  if_icmplt
        ByteHandler::if_icmpge, // = 162:  if_icmpge
        ByteHandler::if_icmpgt, // = 163:  if_icmpgt
        ByteHandler::if_icmple, // = 164:  if_icmple
        ByteHandler::if_acmpeq, // = 165:  if_acmpeq
        ByteHandler::if_acmpne, // = 166:  if_acmpne
        ByteHandler::goto_, // = 167:  goto
        ByteHandler::jsr, // = 168:  jsr
        ByteHandler::ret, // = 169:  ret
        ByteHandler::tableswitch, // = 170:  tableswitch
        ByteHandler::lookupswitch, // = 171:  lookupswitch
        ByteHandler::ireturn, // = 172:  ireturn
        ByteHandler::lreturn, // = 173:  lreturn
        ByteHandler::freturn, // = 174:  freturn
        ByteHandler::dreturn, // = 175:  dreturn
        ByteHandler::areturn, // = 176:  areturn
        ByteHandler::vreturn, // = 177:  return
        ByteHandler::getstatic, // = 178:  getstatic
        ByteHandler::putstatic, // = 179:  putstatic
        ByteHandler::getfield, // = 180:  getfield
        ByteHandler::putfield, // = 181:  putfield
        ByteHandler::invokevirtual, // = 182:  invokevirtual
        ByteHandler::invokespecial, // = 183:  invokespecial
        ByteHandler::invokestatic, // = 184:  invokestatic
        ByteHandler::invokeinterface, // = 185:  invokeinterface
        ByteHandler::invokedynamic, // = 186:  invokedynamic
        ByteHandler::new_, // = 187:  new
        ByteHandler::newarray, // = 188:  newarray
        ByteHandler::anewarray, // = 189:  anewarray
        ByteHandler::arraylength, // = 190:  arraylength
        ByteHandler::athrow, // = 191:  athrow
        ByteHandler::checkcast, // = 192:  checkcast
        ByteHandler::instanceof, // = 193:  instanceof
        ByteHandler::monitorenter, // = 194:  monitorenter
        ByteHandler::monitorexit, // = 195:  monitorexit
        ByteHandler::wide, // = 196:  wide
        ByteHandler::multianewarray, // = 197:  multianewarray
        ByteHandler::ifnull, // = 198:  ifnull
        ByteHandler::ifnonnull, // = 199:  ifnonnull
        ByteHandler::goto_w, // = 200:  goto_w
    };

}