#ifndef JIT_HELP_FUNCTION_HPP
#define JIT_HELP_FUNCTION_HPP
#include <cstdint>

constexpr uint8_t LLVM_COMPILER_INT_ARRAY_TYPE = 0;
constexpr uint8_t LLVM_COMPILER_LONG_ARRAY_TYPE = 1;
constexpr uint8_t LLVM_COMPILER_FLOAT_ARRAY_TYPE = 2;
constexpr uint8_t LLVM_COMPILER_DOUBLE_ARRAY_TYPE = 3;
constexpr uint8_t LLVM_COMPILER_OBJ_ARRAY_TYPE = 4;
constexpr uint8_t LLVM_COMPILER_BYTE_ARRAY_TYPE = 5;
constexpr uint8_t LLVM_COMPILER_CHAR_ARRAY_TYPE = 6;
constexpr uint8_t LLVM_COMPILER_SHORT_ARRAY_TYPE = 7;

constexpr uint8_t LLVM_COMPILER_NEW_DYNAMIC_INVOKE = 0;
constexpr uint8_t LLVM_COMPILER_NEW_OBJECT = 1;
constexpr uint8_t LLVM_COMPILER_NEW_OBJECT_ARRAY = 2;
constexpr uint8_t LLVM_COMPILER_NEW_MULTI_ARRAY = 3;

extern "C" {
    void *llvm_compile_get_string_constant(void *framePtr, uint32_t index);
    void *llvm_compile_get_class_mirror_constant(void *framePtr, uint32_t index);

    void llvm_compile_throw_npe(void *framePtr, uint32_t pc);
    
    int32_t llvm_compile_array_load_i4(void *arrayOop, uint8_t type, uint32_t index);
    int64_t llvm_compile_array_load_i8(void *arrayOop, uint32_t index);
    float llvm_compile_array_load_f4(void *arrayOop, uint32_t index);
    double llvm_compile_array_load_f8(void *arrayOop, uint32_t index);
    void *llvm_compile_array_load_obj(void *arrayOop, uint32_t index);

    void llvm_compile_array_store_i4(void *arrayOop, uint8_t type, uint32_t index, int32_t value);
    void llvm_compile_array_store_i8(void *arrayOop, uint32_t index, int64_t value);
    void llvm_compile_array_store_f4(void *arrayOop, uint32_t index, float value);
    void llvm_compile_array_store_f8(void *arrayOop, uint32_t index, double value);
    void llvm_compile_array_store_obj(void *arrayOop, uint32_t index, void *value);

    void llvm_compile_return_i4(void *framePtr, int32_t val);
    void llvm_compile_return_i8(void *framePtr, int64_t val);
    void llvm_compile_return_f4(void *framePtr, float val);
    void llvm_compile_return_f8(void *framePtr, double val);
    void llvm_compile_return_obj(void *framePtr, void *val);
    void llvm_compile_return_void(void *framePtr);

    void llvm_compile_clinit(void *framePtr, void *klass);
    void *llvm_compile_get_field(void *oop, uint16_t index);
    void llvm_compile_invoke_method(void *framePtr, uint16_t index);
    void llvm_compile_invoke_method_fixed(void *framePtr, void *method, uint16_t paramSize);
    void *llvm_compile_new_object(void *framePtr, uint8_t type, int32_t length, void *klass);
}

#endif