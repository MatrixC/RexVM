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

    void *llvm_compile_get_string_constant(void *framePtr, const uint32_t index) {
        const auto frame = static_cast<Frame *>(framePtr);
        const auto valPtr = frame->constantPool[index].get();
        const auto stringConstInfo = CAST_CONSTANT_STRING_INFO(valPtr);
        const auto strValue = getConstantStringFromPool(frame->constantPool, stringConstInfo->index);
        return frame->mem.getInternString(strValue);
    }


    void *llvm_compile_get_class_mirror_constant(void *framePtr, const uint32_t index) {
        const auto frame = static_cast<Frame *>(framePtr);
        const auto valPtr = frame->constantPool[index].get();
        const auto classConstInfo = CAST_CONSTANT_CLASS_INFO(valPtr);
        const auto className = getConstantStringFromPool(frame->constantPool, classConstInfo->index);
        const auto value = frame->mem.getClass(className);
        return value->getMirror(frame);
    }

    void llvm_compile_throw_npe(void *framePtr, const uint32_t pc) {
        const auto frame = static_cast<Frame *>(framePtr);
        frame->jitPc = pc;
        throwNullPointException(*frame);
    }

    int32_t llvm_compile_array_load_i4(void *arrayOop, const uint8_t type, const uint32_t index) {
        switch (type) {
            case LLVM_COMPILER_INT_ARRAY_TYPE: {
                const auto array = CAST_INT_TYPE_ARRAY_OOP(arrayOop);
                return array->data[index];
            }

            case LLVM_COMPILER_BYTE_ARRAY_TYPE: {
                const auto array = CAST_BYTE_TYPE_ARRAY_OOP(arrayOop);
                return array->data[index];
            }

            case LLVM_COMPILER_CHAR_ARRAY_TYPE: {
                const auto array = CAST_CHAR_TYPE_ARRAY_OOP(arrayOop);
                return array->data[index];
            }

            case LLVM_COMPILER_SHORT_ARRAY_TYPE: {
                const auto array = CAST_SHORT_TYPE_ARRAY_OOP(arrayOop);
                return array->data[index];
            }

            default:
                panic("error");
                return 0;
        }
    }

    int64_t llvm_compile_array_load_i8(void *arrayOop, const uint32_t index) {
        const auto array = CAST_LONG_TYPE_ARRAY_OOP(arrayOop);
        return array->data[index];
    }

    float llvm_compile_array_load_f4(void *arrayOop, const uint32_t index) {
        const auto array = CAST_FLOAT_TYPE_ARRAY_OOP(arrayOop);
        return array->data[index];
    }

    double llvm_compile_array_load_f8(void *arrayOop, const uint32_t index) {
        const auto array = CAST_DOUBLE_TYPE_ARRAY_OOP(arrayOop);
        return array->data[index];
    }

    void *llvm_compile_array_load_obj(void *arrayOop, const uint32_t index) {
        const auto array = CAST_OBJ_ARRAY_OOP(arrayOop);
        return array->data[index];
    }

    void llvm_compile_array_store_i4(void *arrayOop, const uint8_t type, const uint32_t index, const int32_t value) {
        switch (type) {
            case LLVM_COMPILER_INT_ARRAY_TYPE: {
                const auto array = CAST_INT_TYPE_ARRAY_OOP(arrayOop);
                array->data[index] = value;
                break;
            }

            case LLVM_COMPILER_BYTE_ARRAY_TYPE: {
                const auto array = CAST_BYTE_TYPE_ARRAY_OOP(arrayOop);
                array->data[index] = static_cast<u1>(value);
                break;
            }

            case LLVM_COMPILER_CHAR_ARRAY_TYPE: {
                const auto array = CAST_CHAR_TYPE_ARRAY_OOP(arrayOop);
                array->data[index] = static_cast<cchar_16>(value);
                break;
            }

            case LLVM_COMPILER_SHORT_ARRAY_TYPE: {
                const auto array = CAST_SHORT_TYPE_ARRAY_OOP(arrayOop);
                array->data[index] = static_cast<i2>(value);
                break;
            }

            default:
                panic("error");
        }
    }

    void llvm_compile_array_store_i8(void *arrayOop, const uint32_t index, const int64_t value) {
        const auto array = CAST_LONG_TYPE_ARRAY_OOP(arrayOop);
        array->data[index] = value;
    }

    void llvm_compile_array_store_f4(void *arrayOop, const uint32_t index, const float value) {
        const auto array = CAST_FLOAT_TYPE_ARRAY_OOP(arrayOop);
        array->data[index] = value;
    }

    void llvm_compile_array_store_f8(void *arrayOop, const uint32_t index, const double value) {
        const auto array = CAST_DOUBLE_TYPE_ARRAY_OOP(arrayOop);
        array->data[index] = value;
    }

    void llvm_compile_array_store_obj(void *arrayOop, const uint32_t index, void *value) {
        const auto array = CAST_OBJ_ARRAY_OOP(arrayOop);
        array->data[index] = static_cast<ref>(value);
    }

    void llvm_compile_return_i4(void *framePtr, const int32_t val) {
        const auto frame = static_cast<Frame *>(framePtr);
        frame->returnI4(val);
    }

    void llvm_compile_return_i8(void *framePtr, const int64_t val) {
        const auto frame = static_cast<Frame *>(framePtr);
        frame->returnI8(val);
    }

    void llvm_compile_return_f4(void *framePtr, const float val) {
        const auto frame = static_cast<Frame *>(framePtr);
        frame->returnF4(val);
    }

    void llvm_compile_return_f8(void *framePtr, const double val) {
        const auto frame = static_cast<Frame *>(framePtr);
        frame->returnF8(val);
    }

    void llvm_compile_return_obj(void *framePtr, void *val) {
        const auto frame = static_cast<Frame *>(framePtr);
        frame->returnRef(CAST_REF(val));
    }

    void llvm_compile_return_void(void *framePtr) {
        const auto frame = static_cast<Frame *>(framePtr);
        frame->returnVoid();
    }

    void llvm_compile_clinit(void *framePtr, void *klass) {
        const auto frame = static_cast<Frame *>(framePtr);
        const auto instanceClass = CAST_INSTANCE_CLASS(klass);
        instanceClass->clinit(*frame);
    }

    void *llvm_compile_get_field(void *oop, const uint16_t index) {
        const auto instanceOop = CAST_INSTANCE_OOP(oop);
        return instanceOop->data.get() + index;
    }

    void llvm_compile_invoke_method(void *framePtr, const uint16_t index) {
        const auto frame = static_cast<Frame *>(framePtr);
        auto &operandStack = frame->operandStackContext;

        const auto cache = frame->mem.resolveInvokeVirtualIndex(index, false);
        const auto paramSize = cache->paramSlotSize;
        operandStack.sp += CAST_I4(paramSize);

        const auto instance = frame->getStackOffset(paramSize - 1).refVal;
        const auto instanceClass = CAST_INSTANCE_CLASS(instance->getClass());
        const auto invokeMethod = frame->mem.linkVirtualMethod(index, cache->methodName, cache->methodDescriptor, instanceClass);
        frame->runMethodInner(*invokeMethod);

        operandStack.pop(getSlotTypeStoreCount(getSlotTypeByPrimitiveClassName(invokeMethod->returnType)));
    }

    void llvm_compile_invoke_method_fixed(void *framePtr, void *method, const uint16_t paramSize) {
        const auto frame = static_cast<Frame *>(framePtr);
        auto &operandStack = frame->operandStackContext;
        operandStack.sp += CAST_I4(paramSize);

        const auto invokeMethod = static_cast<Method *>(method);
        invokeMethod->klass.clinit(*frame);
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

        switch (type) {
            case LLVM_COMPILER_NEW_DYNAMIC_INVOKE: {
                return invokeDynamic(*frame, length);
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

}
