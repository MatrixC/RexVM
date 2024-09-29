#include "jit_help_function.hpp"
#include <bit>
#include "../config.hpp"
#include "../frame.hpp"
#include "../class.hpp"
#include "../class_member.hpp"
#include "../oop.hpp"
#include "../constant_info.hpp"
#include "../class_loader.hpp"
#include "../exception_helper.hpp"

extern "C" {

    using namespace RexVM;

    void *llvm_compile_get_string_constant(void *framePtr, uint32_t index) {
        const auto frame = static_cast<Frame *>(framePtr);
        const auto valPtr = frame->constantPool[index].get();
        const auto stringConstInfo = CAST_CONSTANT_STRING_INFO(valPtr);
        const auto strValue = getConstantStringFromPool(frame->constantPool, stringConstInfo->index);
        return frame->mem.getInternString(strValue);
    }


    void *llvm_compile_get_class_mirror_constant(void *framePtr, uint32_t index) {
        const auto frame = static_cast<RexVM::Frame *>(framePtr);
        const auto valPtr = frame->constantPool[index].get();
        const auto classConstInfo = CAST_CONSTANT_CLASS_INFO(valPtr);
        const auto className = getConstantStringFromPool(frame->constantPool, classConstInfo->index);
        const auto value = frame->mem.getClass(className);
        return value->getMirror(frame);
    }

    void llvm_compile_throw_npe(void *framePtr) {
        const auto frame = static_cast<RexVM::Frame *>(framePtr);
        throwNullPointException(*frame);
    }

    uint64_t floatToU64(float x) {
        union Tran {
            uint64_t v1{0};
            float v2;
        };
        Tran t;
        t.v2 = x;
        return t.v1;
    }

    int32_t llvm_compile_array_load_i4(void *arrayOop, uint8_t type, uint32_t index) {
        switch (type) {
            case LLVM_COMPILER_INT_ARRAY_TYPE: {
                const auto array = CAST_INT_TYPE_ARRAY_OOP(arrayOop);
                return static_cast<int32_t>(array->data[index]);
            }

            case LLVM_COMPILER_BYTE_ARRAY_TYPE: {
                const auto array = CAST_BYTE_TYPE_ARRAY_OOP(arrayOop);
                return static_cast<int32_t>(array->data[index]);
            }

            case LLVM_COMPILER_CHAR_ARRAY_TYPE: {
                const auto array = CAST_CHAR_TYPE_ARRAY_OOP(arrayOop);
                return static_cast<int32_t>(array->data[index]);
            }

            case LLVM_COMPILER_SHORT_ARRAY_TYPE: {
                const auto array = CAST_SHORT_TYPE_ARRAY_OOP(arrayOop);
                return static_cast<int32_t>(array->data[index]);
            }

            default:
                panic("error");
        }
    }

    int64_t llvm_compile_array_load_i8(void *arrayOop, uint32_t index) {
        const auto array = CAST_LONG_TYPE_ARRAY_OOP(arrayOop);
        return static_cast<uint64_t>(array->data[index]);
    }

    float llvm_compile_array_load_f4(void *arrayOop, uint32_t index) {
        const auto array = CAST_FLOAT_TYPE_ARRAY_OOP(arrayOop);
        return floatToU64(array->data[index]);
    }

    double llvm_compile_array_load_f8(void *arrayOop, uint32_t index) {
        const auto array = CAST_DOUBLE_TYPE_ARRAY_OOP(arrayOop);
        return std::bit_cast<uint64_t>(array->data[index]);
    }

    void *llvm_compile_array_load_obj(void *arrayOop, uint32_t index) {
        const auto array = CAST_OBJ_ARRAY_OOP(arrayOop);
        return std::bit_cast<void *>(array->data[index]);
    }

    void llvm_compile_array_store_i4(void *arrayOop, uint8_t type, uint32_t index, int32_t value) {
        switch (type) {
            case LLVM_COMPILER_INT_ARRAY_TYPE: {
                const auto array = CAST_INT_TYPE_ARRAY_OOP(arrayOop);
                array->data[index] = static_cast<i4>(value);
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
        }
    }

    void llvm_compile_array_store_i8(void *arrayOop, uint32_t index, int64_t value) {
        const auto array = CAST_LONG_TYPE_ARRAY_OOP(arrayOop);
        array->data[index] = value;
    }

    void llvm_compile_array_store_f4(void *arrayOop, uint32_t index, float value) {
        const auto array = CAST_FLOAT_TYPE_ARRAY_OOP(arrayOop);
        array->data[index] = value;
    }

    void llvm_compile_array_store_f8(void *arrayOop, uint32_t index, double value) {
        const auto array = CAST_DOUBLE_TYPE_ARRAY_OOP(arrayOop);
        array->data[index] = value;
    }

    void llvm_compile_array_store_obj(void *arrayOop, uint32_t index, void *value) {
        const auto array = CAST_OBJ_ARRAY_OOP(arrayOop);
        array->data[index] = static_cast<ref>(value);
    }

    void llvm_compile_return_i4(void *framePtr, int32_t val) {
        const auto frame = static_cast<Frame *>(framePtr);
        frame->returnI4(val);
    }

    void llvm_compile_return_i8(void *framePtr, int64_t val) {
        const auto frame = static_cast<Frame *>(framePtr);
        frame->returnI8(val);
    }

    void llvm_compile_return_f4(void *framePtr, float val) {
        const auto frame = static_cast<Frame *>(framePtr);
        frame->returnF4(val);
    }

    void llvm_compile_return_f8(void *framePtr, double val) {
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



}
