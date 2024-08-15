#include "basic_java_class.hpp"

namespace RexVM {

    const std::array<cstring, BASIC_JAVA_CLASS_ENUM_COUNT> BASIC_JAVA_CLASS_NAMES = {
            JAVA_LANG_OBJECT_NAME,
            JAVA_LANG_CLASS_NAME,
            JAVA_LANG_CLASS_LOADER_NAME,
            JAVA_LANG_SYSTEM_NAME,
            JAVA_LANG_THREAD_NAME,
            JAVA_LANG_THREAD_GROUP_NAME,
            JAVA_LANG_BOOLEAN_NAME,
            JAVA_LANG_BYTE_NAME,
            JAVA_LANG_CHARACTER_NAME,
            JAVA_LANG_SHORT_NAME,
            JAVA_LANG_INTEGER_NAME,
            JAVA_LANG_LONG_NAME,
            JAVA_LANG_FLOAT_NAME,
            JAVA_LANG_DOUBLE_NAME,
            JAVA_LANG_STRING_NAME,
            JAVA_LANG_ENUM_NAME,
            JAVA_LANG_EXCEPTION_NAME,
            JAVA_LANG_THROWABLE_NAME,
            JAVA_LANG_CLONEABLE_NAME,
            JAVA_IO_SERIALIZABLE_NAME,
            JAVA_LANG_RUNTIME_NAME,
            JAVA_LANG_INVOKE_METHOD_HANDLE_NAME,
            JAVA_LANG_STACK_TRACE_ELEMENT_NAME,
            JAVA_LANG_REFLECT_METHOD_NAME,
            JAVA_LANG_REFLECT_FIELD_NAME,
            JAVA_LANG_INVOKE_MEMBER_NAME_NAME,
    };

    cstring getBasicJavaClassName(BasicJavaClassEnum classEnum) {
        return BASIC_JAVA_CLASS_NAMES[CAST_SIZE_T(classEnum)];
    }

}