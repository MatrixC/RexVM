#include "jit_help_function.hpp"
#include "../config.hpp"
#include "../frame.hpp"
#include "../class.hpp"
#include "../class_member.hpp"
#include "../oop.hpp"
#include "../constant_info.hpp"
#include "../exception_helper.hpp"

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

    void llvm_compile_throw_npe(void *framePtr) {
        //TODO set frame pc
        const auto frame = static_cast<Frame *>(framePtr);
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
        const auto instanceClass = static_cast<InstanceClass *>(klass);
        instanceClass->clinit(*frame);
    }

}
