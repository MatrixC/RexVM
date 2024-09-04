#ifndef BASIC_JAVA_CLASS_HPP
#define BASIC_JAVA_CLASS_HPP

#include <array>
#include "config.hpp"
#include "composite_ptr.hpp"

namespace RexVM {

    inline const cview JAVA_LANG_OBJECT_NAME = "java/lang/Object";
    inline const cview JAVA_LANG_CLASS_NAME = "java/lang/Class";
    inline const cview JAVA_LANG_CLASS_LOADER_NAME = "java/lang/ClassLoader";
    inline const cview JAVA_LANG_SYSTEM_NAME = "java/lang/System";
    inline const cview JAVA_LANG_THREAD_NAME = "java/lang/Thread";
    inline const cview JAVA_LANG_THREAD_GROUP_NAME = "java/lang/ThreadGroup";
    inline const cview JAVA_LANG_BOOLEAN_NAME = "java/lang/Boolean";
    inline const cview JAVA_LANG_BYTE_NAME = "java/lang/Byte";
    inline const cview JAVA_LANG_CHARACTER_NAME = "java/lang/Character";
    inline const cview JAVA_LANG_SHORT_NAME = "java/lang/Short";
    inline const cview JAVA_LANG_INTEGER_NAME = "java/lang/Integer";
    inline const cview JAVA_LANG_LONG_NAME = "java/lang/Long";
    inline const cview JAVA_LANG_FLOAT_NAME = "java/lang/Float";
    inline const cview JAVA_LANG_DOUBLE_NAME = "java/lang/Double";
    inline const cview JAVA_LANG_STRING_NAME = "java/lang/String";
    inline const cview JAVA_LANG_ENUM_NAME = "java/lang/Enum";
    inline const cview JAVA_LANG_EXCEPTION_NAME = "java/lang/Exception";
    inline const cview JAVA_LANG_THROWABLE_NAME = "java/lang/Throwable";
    inline const cview JAVA_LANG_CLONEABLE_NAME = "java/lang/Cloneable";
    inline const cview JAVA_IO_SERIALIZABLE_NAME = "java/io/Serializable";
    inline const cview JAVA_LANG_RUNTIME_NAME = "java/lang/Runtime";
    inline const cview JAVA_LANG_INVOKE_METHOD_HANDLE_NAME = "java/lang/invoke/MethodHandle";
    inline const cview JAVA_LANG_STACK_TRACE_ELEMENT_NAME = "java/lang/StackTraceElement";
    inline const cview JAVA_LANG_REFLECT_METHOD_NAME = "java/lang/reflect/Method";
    inline const cview JAVA_LANG_REFLECT_CONSTRUCTOR_NAME = "java/lang/reflect/Constructor";
    inline const cview JAVA_LANG_REFLECT_FIELD_NAME = "java/lang/reflect/Field";
    inline const cview JAVA_LANG_INVOKE_MEMBER_NAME_NAME = "java/lang/invoke/MemberName";
    inline const cview SUN_REFLECT_CONSTANT_POOL_NAME = "sun/reflect/ConstantPool";

    enum class BasicJavaClassEnum : size_t {
        JAVA_LANG_OBJECT,
        JAVA_LANG_CLASS,
        JAVA_LANG_CLASS_LOADER,
        JAVA_LANG_SYSTEM,
        JAVA_LANG_THREAD,
        JAVA_LANG_THREAD_GROUP,
        JAVA_LANG_BOOLEAN,
        JAVA_LANG_BYTE,
        JAVA_LANG_CHARACTER,
        JAVA_LANG_SHORT,
        JAVA_LANG_INTEGER,
        JAVA_LANG_LONG,
        JAVA_LANG_FLOAT,
        JAVA_LANG_DOUBLE,
        JAVA_LANG_STRING,
        JAVA_LANG_ENUM,
        JAVA_LANG_EXCEPTION,
        JAVA_LANG_THROWABLE,
        JAVA_LANG_CLONEABLE,
        JAVA_IO_SERIALIZABLE,
        JAVA_LANG_RUNTIME,
        JAVA_LANG_INVOKE_METHOD_HANDLE,
        JAVA_LANG_STACK_TRACE_ELEMENT,
        JAVA_LANG_REFLECT_METHOD,
        JAVA_LANG_REFLECT_CONSTRUCTOR,
        JAVA_LANG_REFLECT_FIELD,
        JAVA_LANG_INVOKE_MEMBER_NAME,
        SUN_REFLECT_CONSTANT_POOL,
        COUNT,
    };
    constexpr auto BASIC_JAVA_CLASS_ENUM_COUNT = CAST_SIZE_T(BasicJavaClassEnum::COUNT) + 1;

    extern const std::array<cview, BASIC_JAVA_CLASS_ENUM_COUNT> BASIC_JAVA_CLASS_NAMES;

    cview getBasicJavaClassName(BasicJavaClassEnum classEnum);
}

#endif