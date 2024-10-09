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

constexpr uint8_t LLVM_COMPILER_CHECK_CAST = 0;
constexpr uint8_t LLVM_COMPILER_INSTANCE_OF = 1;

constexpr uint8_t LLVM_COMPILER_MONITOR_ENTER = 0;
constexpr uint8_t LLVM_COMPILER_MONITOR_EXIT = 1;

constexpr uint8_t LLVM_COMPILER_FIXED_EXCEPTION_NPE = 0;
constexpr uint8_t LLVM_COMPILER_FIXED_EXCEPTION_DIV_BY_ZERO = 1;

extern "C" {
    void *llvm_compile_get_instance_constant(void *framePtr, uint32_t index);

    int32_t llvm_compile_array_length(void *arrayOop);

    void llvm_compile_return_common(void *framePtr, int64_t val, uint8_t type);

    void llvm_compile_clinit(void *framePtr, void *klass);

    void *llvm_compile_invoke_method_fixed(void *framePtr, void *method, uint16_t paramSize);

    void *llvm_compile_new_object(void *framePtr, uint8_t type, int32_t length, void *klass);

    void llvm_compile_throw_exception(void *framePtr, void *exOop, uint32_t pc, uint8_t fixedException);

    int32_t llvm_compile_check_cast(void *framePtr, uint8_t type, void *popOop, void *check);

    void llvm_compile_monitor(void *oop, uint8_t type);
}

#endif