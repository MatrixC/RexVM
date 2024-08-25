#include "native_rex.hpp"
#include "rex_print.hpp"

namespace RexVM::Native::Rex {

    constexpr auto REX_PRINT_STREAM_CLASS_NAME = "RexPrintStream";

    void registerRexMethods(NativeManager &manager) {
        manager.regNativeMethod(REX_PRINT_STREAM_CLASS_NAME, "print", "(Z)V", false, RexVM::Native::Rex::print_boolean);
        manager.regNativeMethod(REX_PRINT_STREAM_CLASS_NAME, "print", "(C)V", false, RexVM::Native::Rex::print_char);
        manager.regNativeMethod(REX_PRINT_STREAM_CLASS_NAME, "print", "(I)V", false, RexVM::Native::Rex::print_int);
        manager.regNativeMethod(REX_PRINT_STREAM_CLASS_NAME, "print", "(J)V", false, RexVM::Native::Rex::print_long);
        manager.regNativeMethod(REX_PRINT_STREAM_CLASS_NAME, "print", "(F)V", false, RexVM::Native::Rex::print_float);
        manager.regNativeMethod(REX_PRINT_STREAM_CLASS_NAME, "print", "(D)V", false, RexVM::Native::Rex::print_double);
        manager.regNativeMethod(REX_PRINT_STREAM_CLASS_NAME, "print", "([C)V", false, RexVM::Native::Rex::print_char_arr);
        manager.regNativeMethod(REX_PRINT_STREAM_CLASS_NAME, "print", "(Ljava/lang/String;)V", false, RexVM::Native::Rex::print_String);
        manager.regNativeMethod(REX_PRINT_STREAM_CLASS_NAME, "print", "(Ljava/lang/Object;)V", false, RexVM::Native::Rex::print_Object);
        manager.regNativeMethod(REX_PRINT_STREAM_CLASS_NAME, "println", "(Z)V", false, RexVM::Native::Rex::println_boolean);
        manager.regNativeMethod(REX_PRINT_STREAM_CLASS_NAME, "println", "(C)V", false, RexVM::Native::Rex::println_char);
        manager.regNativeMethod(REX_PRINT_STREAM_CLASS_NAME, "println", "(I)V", false, RexVM::Native::Rex::println_int);
        manager.regNativeMethod(REX_PRINT_STREAM_CLASS_NAME, "println", "(J)V", false, RexVM::Native::Rex::println_long);
        manager.regNativeMethod(REX_PRINT_STREAM_CLASS_NAME, "println", "(F)V", false, RexVM::Native::Rex::println_float);
        manager.regNativeMethod(REX_PRINT_STREAM_CLASS_NAME, "println", "(D)V", false, RexVM::Native::Rex::println_double);
        manager.regNativeMethod(REX_PRINT_STREAM_CLASS_NAME, "println", "([C)V", false, RexVM::Native::Rex::println_char_arr);
        manager.regNativeMethod(REX_PRINT_STREAM_CLASS_NAME, "println", "(Ljava/lang/String;)V", false, RexVM::Native::Rex::println_String);
        manager.regNativeMethod(REX_PRINT_STREAM_CLASS_NAME, "println", "(Ljava/lang/Object;)V", false, RexVM::Native::Rex::println_Object);
        manager.regNativeMethod(REX_PRINT_STREAM_CLASS_NAME, "println", "()V", false, RexVM::Native::Rex::println_empty);
    }

}
