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

    void *llvm_compile_invoke_method_fixed(void *framePtr, void *method, uint16_t paramSize, const uint32_t pc, const int8_t isClinit) {
        const auto frame = static_cast<Frame *>(framePtr);
        frame->pcCode = CAST_I4(pc);

        if (isClinit != 0) {
            const auto instanceClass = CAST_INSTANCE_CLASS(method);
            instanceClass->clinit(*frame);
            if (frame->markThrow) {
                return frame->throwValue;
            }
            return nullptr;
        }

        auto &operandStack = frame->operandStackContext;
        Method *invokeMethod{nullptr};
        if (method != nullptr) {
            operandStack.sp += CAST_I4(paramSize);
            invokeMethod = static_cast<Method *>(method);
            invokeMethod->klass.clinit(*frame);
            if (frame->markThrow) {
                return frame->throwValue;
            }
        } else {
            const auto index = paramSize;
            const auto cache = frame->mem.resolveInvokeVirtualIndex(index, false);
            paramSize = cache->paramSlotSize;
            operandStack.sp += CAST_I4(paramSize);

            const auto instance = frame->getStackOffset(paramSize - 1).refVal;
            const auto instanceClass = CAST_INSTANCE_CLASS(instance->getClass());
            invokeMethod = frame->mem.linkVirtualMethod(index, cache->methodName, cache->methodDescriptor, instanceClass);
        }

        if (isMethodHandleInvoke(invokeMethod->klass.getClassName(), invokeMethod->getName())) {
            frame->runMethodInner(*invokeMethod, paramSize);
        } else {
            frame->runMethodInner(*invokeMethod);
        }

        if (frame->markThrow) {
            return frame->throwValue;
        }


        if (invokeMethod->slotType == SlotTypeEnum::REF) {
            //如果有返回值 很关键 否则gc root 扫描不到jit函数调用其他函数的返回值 会被直接回收
            if (const auto returnValue = frame->operandStackContext.top().refVal; returnValue != nullptr) {
                frame->addCreateRef(returnValue);
            }
        }

        operandStack.sp = -1;
        return nullptr;
    }

    void *llvm_compile_new_object(void *framePtr, uint8_t type, const int32_t length, void *klass, int32_t *exception) {
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
                //这里跟llvm_compile_invoke_method_fixed一样 因为调用了pushParam传参 所以这里需要将传参的sp加上去
                operandStack.sp += CAST_I4(paramSize);
                const auto callSiteObj = frame->mem.invokeDynamic(index);
                if (frame->markThrow) {
                    *exception = 1;
                    return frame->throwValue;
                }
                return callSiteObj;
            }

            case LLVM_COMPILER_NEW_OBJECT: {
                const auto instanceClass = CAST_INSTANCE_CLASS(klass);
                instanceClass->clinit(*frame);
                if (frame->markThrow) {
                    *exception = 1;
                    return frame->throwValue;
                }
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

    void llvm_compile_throw_exception(
        void *framePtr,
        void *exOop,
        const uint32_t pc,
        const uint8_t fixedException,
        void *exField
    ) {
        const auto frame = static_cast<Frame *>(framePtr);
        frame->pcCode = CAST_I4(pc);

        if (fixedException == LLVM_COMPILER_FIXED_EXCEPTION_NPE) {
            throwNullPointException(*frame);
            return;
        }

        if (fixedException == LLVM_COMPILER_FIXED_EXCEPTION_DIV_BY_ZERO) {
            throwArithmeticExceptionDivByZero(*frame);
            return;
        }

        if (fixedException == LLVM_COMPILER_FIXED_EXCEPTION_CLASS_CHECK) {
            const auto refVal = CAST_REF(exOop);
            const auto checkClass = CAST_CLASS(exField);
            throwClassCastException(*frame, refVal->getClass()->getClassName(), checkClass->getClassName());
            return;
        }

        if (exOop == nullptr) {
            panic("exOop can't be null");
        }

        frame->throwException(CAST_INSTANCE_OOP(exOop));
    }

    int32_t llvm_compile_match_catch(void *oop, void **catchClassArray, const int32_t size) {
        const auto exOop = CAST_REF(oop);
        const auto exClass = exOop->getClass();

        for (int32_t i = 0; i < size; ++i) {
            const auto cPtr = catchClassArray[i];
            const auto catchClass = CAST_INSTANCE_CLASS(cPtr);
            if (exClass == catchClass || catchClass->isSuperClassOf(exClass)) {
               return i;
            }
        }

        return -1;
    }

    int32_t llvm_compile_misc(void *framePtr, void *pa, void *pb, const uint8_t type) {
        switch (type) {
            case LLVM_COMPILER_MISC_MONITOR_ENTER: {
                const auto refVal = CAST_REF(pa);
                refVal->lock();
                return 0;
            }

            case LLVM_COMPILER_MISC_MONITOR_EXIT: {
                const auto refVal = CAST_REF(pa);
                refVal->unlock();
                return 0;
            }

            case LLVM_COMPILER_MISC_ARRAY_LENGTH: {
                const auto array = CAST_ARRAY_OOP(pa);
                return static_cast<int32_t>(array->getDataLength());
            }

            case LLVM_COMPILER_MISC_CHECK_INSTANCE_OF: {
                if (pa == nullptr) {
                    //兼容instanceOf指令 checkClass指令肯定不会传null
                    //这样可以让instanceOf的JIT少一步判断 直接返回
                    return 0;
                }
                const auto ppa = CAST_REF(pa);
                const auto ppb = CAST_CLASS(pb);
                const auto instanceOf = ppa->isInstanceOf(ppb);
                return instanceOf ? 1 : 0;
            }

            case LLVM_COMPILER_MISC_CLEAN_THROW: {
                const auto frame = static_cast<Frame *>(framePtr);
                frame->cleanOperandStack();
                frame->cleanThrow();
                return 0;
            }

            default:
                panic("error type");
        }

        return -1;
    }

}
