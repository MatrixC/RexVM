#include "jit_help_function.hpp"
#include "../config.hpp"
#include "../frame.hpp"
#include "../class.hpp"
#include "../class_member.hpp"
#include "../oop.hpp"
#include "../constant_info.hpp"
#include "../exception_helper.hpp"
#include "../method_handle.hpp"

extern "C" {

    using namespace RexVM;

    void *llvm_compile_get_instance_constant(void *framePtr, const uint32_t index) {
        const auto frame = static_cast<Frame *>(framePtr);
        const auto valPtr = frame->constantPool[index].get();
        const auto constantTagEnum = CAST_CONSTANT_TAG_ENUM(valPtr->tag);
        if (constantTagEnum == ConstantTagEnum::CONSTANT_String) {
            const auto stringConstInfo = CAST_CONSTANT_STRING_INFO(valPtr);
            const auto strValue = getConstantStringFromPool(frame->constantPool, stringConstInfo->index);
            return frame->mem.getInternString(strValue);
        }
        if (constantTagEnum == ConstantTagEnum::CONSTANT_Class) {
            const auto classConstInfo = CAST_CONSTANT_CLASS_INFO(valPtr);
            const auto className = getConstantStringFromPool(frame->constantPool, classConstInfo->index);
            const auto value = frame->mem.getClass(className);
            return value->getMirror(frame);
        }
        panic("error tag");
        return nullptr;
    }

    int32_t llvm_compile_array_length(void *arrayOop) {
        const auto array = CAST_ARRAY_OOP(arrayOop);
        return static_cast<int32_t>(array->getDataLength());
    }

    void llvm_compile_return_common(void *framePtr, const int64_t val, uint8_t type) {
        const auto frame = static_cast<Frame *>(framePtr);
        switch (static_cast<SlotTypeEnum>(type)) {
            case SlotTypeEnum::NONE:
                frame->returnVoid();
                break;

            case SlotTypeEnum::I4:
                frame->returnI4(CAST_I4(val));
                break;

            case SlotTypeEnum::I8:
                frame->returnI8(val);
                break;

            case SlotTypeEnum::F4: {
                const auto int32Val = CAST_I4(val);
                frame->returnF4(std::bit_cast<f4>(int32Val));
                break;
            }

            case SlotTypeEnum::F8:
                frame->returnF8(std::bit_cast<f8>(val));
                break;

            case SlotTypeEnum::REF:
                frame->returnRef(std::bit_cast<ref>(val));
                break;
        }
    }

    void llvm_compile_clinit(void *framePtr, void *klass) {
        const auto frame = static_cast<Frame *>(framePtr);
        const auto instanceClass = CAST_INSTANCE_CLASS(klass);
        instanceClass->clinit(*frame);
    }

    void llvm_compile_invoke_method_fixed(void *framePtr, void *method, const uint16_t paramSize) {
        const auto frame = static_cast<Frame *>(framePtr);
        auto &operandStack = frame->operandStackContext;

        Method *invokeMethod{nullptr};
        if (method != nullptr) {
            operandStack.sp += CAST_I4(paramSize);
            invokeMethod = static_cast<Method *>(method);
            invokeMethod->klass.clinit(*frame);
        } else {
            const auto index = paramSize;
            const auto cache = frame->mem.resolveInvokeVirtualIndex(index, false);
            operandStack.sp += CAST_I4( cache->paramSlotSize);

            const auto instance = frame->getStackOffset( cache->paramSlotSize - 1).refVal;
            const auto instanceClass = CAST_INSTANCE_CLASS(instance->getClass());
            invokeMethod = frame->mem.linkVirtualMethod(index, cache->methodName, cache->methodDescriptor, instanceClass);
        }

        frame->runMethodInner(*invokeMethod);
        operandStack.pop(getSlotTypeStoreCount(getSlotTypeByPrimitiveClassName(invokeMethod->returnType)));
    }

    void *llvm_compile_new_object(void *framePtr, uint8_t type, const int32_t length, void *klass) {
        //type
        //0: dynamicInvoke
        //1: newObject
        //2: newObjectArray
        //3: newMultiArray
        //4-13: newArray
        const auto frame = static_cast<Frame *>(framePtr);
        auto &operandStack = frame->operandStackContext;

        switch (type) {
            case LLVM_COMPILER_NEW_DYNAMIC_INVOKE: {
                const auto index = CAST_U2(length & 0xFFFF);
                const auto paramSize = CAST_U2((length >> 16) & 0xFFFF);
                operandStack.sp += CAST_I4(paramSize);
                const auto callSiteObj = invokeDynamic(*frame, index);
                //operandStack.pop(1);
                return callSiteObj;
            }

            case LLVM_COMPILER_NEW_OBJECT: {
                const auto instanceClass = CAST_INSTANCE_CLASS(klass);
                instanceClass->clinit(*frame);
                return frame->mem.newInstance(instanceClass);
            }

            case LLVM_COMPILER_NEW_OBJECT_ARRAY: {
                const auto arrayClass = CAST_OBJ_ARRAY_CLASS(klass);
                return frame->mem.newObjArrayOop(arrayClass, length);
            }

            case LLVM_COMPILER_NEW_MULTI_ARRAY: {
                const auto index = CAST_U2(length & 0xFFFF);
                const auto dimCount = CAST_U2((length >> 16) & 0xFFFF);
                const auto dimLength = static_cast<i4 *>(klass);
                return frame->mem.newMultiArrayOop(index, dimLength, CAST_I2(dimCount));
            }

            default: {
                if (type > 13) {
                    panic("type error");
                    return nullptr;
                }
                const auto basicType = static_cast<BasicType>(type);
                return frame->mem.newTypeArrayOop(basicType, length);
            }
        }
    }

    void llvm_compile_throw_exception(void *framePtr, void *exOop, const uint32_t pc, const uint8_t fixedException) {
        const auto frame = static_cast<Frame *>(framePtr);
        frame->jitPc = pc;

        if (exOop == nullptr) {
            if (fixedException == LLVM_COMPILER_FIXED_EXCEPTION_NPE) {
                throwNullPointException(*frame);
                return;
            }

            if (fixedException == LLVM_COMPILER_FIXED_EXCEPTION_DIV_BY_ZERO) {
                throwArithmeticExceptionDivByZero(*frame);
                return;
            }
        }

        frame->throwException(CAST_INSTANCE_OOP(exOop));
    }

    int32_t llvm_compile_check_cast(void *framePtr, const uint8_t type, void *popOop, void *check) {
        const auto frame = static_cast<Frame *>(framePtr);
        const auto refVal = CAST_REF(popOop);
        const auto checkClass = CAST_CLASS(check);
        if (refVal == nullptr) {
            if (type == LLVM_COMPILER_CHECK_CAST) {
                //adapt checkcast opcode, return 1 don't exit method
                return 1;
            } else {
                //adapt instaneof opcode, return 0 isn't instanceOf
                return 0;
            }
        }

        const auto instanceOf = refVal->isInstanceOf(checkClass);
        const int32_t result = instanceOf ? 1 : 0;

        if (type == LLVM_COMPILER_CHECK_CAST && !instanceOf) {
            throwClassCastException(*frame, refVal->getClass()->getClassName(), checkClass->getClassName());
        }

        return result;
    }

    void llvm_compile_monitor(void *oop, const uint8_t type) {
        const auto refVal = CAST_REF(oop);
        if (type == LLVM_COMPILER_MONITOR_ENTER) {
            refVal->lock();
        } else {
            refVal->unlock();
        }
    }



}
