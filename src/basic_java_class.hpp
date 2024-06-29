#ifndef BASIC_JAVA_CLASS_HPP
#define BASIC_JAVA_CLASS_HPP
#include <unordered_map>
#include <array>
#include "config.hpp"

namespace RexVM {

    inline const cstring JAVA_LANG_OBJECT_NAME =         "java/lang/Object";
    inline const cstring JAVA_LANG_CLASS_NAME =         "java/lang/Class";
    inline const cstring JAVA_LANG_CLASS_LOADER_NAME =         "java/lang/ClassLoader";
    inline const cstring JAVA_LANG_SYSTEM_NAME =         "java/lang/System";
    inline const cstring JAVA_LANG_THREAD_NAME =         "java/lang/Thread";
    inline const cstring JAVA_LANG_THREAD_GROUP_NAME =         "java/lang/ThreadGroup";
    inline const cstring JAVA_LANG_BOOLEAN_NAME =         "java/lang/Boolean";
    inline const cstring JAVA_LANG_CHARACTER_NAME =         "java/lang/Character";
    inline const cstring JAVA_LANG_SHORT_NAME =         "java/lang/Short";
    inline const cstring JAVA_LANG_INTEGER_NAME =         "java/lang/Integer";
    inline const cstring JAVA_LANG_LONG_NAME =         "java/lang/Long";
    inline const cstring JAVA_LANG_FLOAT_NAME =         "java/lang/Float";
    inline const cstring JAVA_LANG_DOUBLE_NAME =         "java/lang/Double";
    inline const cstring JAVA_LANG_STRING_NAME =         "java/lang/String";
    inline const cstring JAVA_LANG_ENUM_NAME =         "java/lang/Enum";
    inline const cstring JAVA_LANG_EXCEPTION_NAME =         "java/lang/Exception";
    inline const cstring JAVA_LANG_THROWABLE_NAME = "java/lang/Throwable";
    inline const cstring JAVA_LANG_CLONEABLE_NAME = "java/lang/Cloneable";
    inline const cstring JAVA_IO_SERIALIZABLE_NAME = "java/io/Serializable";

    enum class BasicJavaClassEnum : size_t {
        JAVA_LANG_OBJECT,
        JAVA_LANG_CLASS,
        JAVA_LANG_CLASS_LOADER,
        JAVA_LANG_SYSTEM,
        JAVA_LANG_THREAD,
        JAVA_LANG_THREAD_GROUP,
        JAVA_LANG_BOOLEAN,
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
        COUNT,
    };
    constexpr auto BASIC_JAVA_CLASS_ENUM_COUNT = static_cast<size_t>(BasicJavaClassEnum::COUNT) + 1;

    extern const std::array<cstring, BASIC_JAVA_CLASS_ENUM_COUNT> BASIC_JAVA_CLASS_NAMES;

    cstring getBasicJavaClassName(BasicJavaClassEnum classEnum);
}

#endif