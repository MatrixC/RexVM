#ifndef CLASS_UTIL_HPP
#define CLASS_UTIL_HPP
#include "../config.hpp"

namespace RexVM {

    class Oop;
    struct ClassLoader;
    struct ByteTypeArrayOop;
    struct Class;

    cstring getDescriptorClassName(cview className);
    cstring getClassNameByFieldDescriptor(cview descriptor);
    cstring getJVMClassName(const cstring &javaClassName);
    cstring getJavaClassName(const cstring &jvmClassName);
    bool isWideClassName(const cstring &className);
}

#endif
