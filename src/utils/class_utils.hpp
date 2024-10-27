#ifndef CLASS_UTIL_HPP
#define CLASS_UTIL_HPP
#include "../basic.hpp"

namespace RexVM {

    struct Oop;
    struct ClassLoader;
    struct ByteTypeArrayOop;
    struct Class;

    cview getClassNameByFieldDescriptor(cview descriptor);
    cstring getJVMClassName(cview javaClassName);
    cstring getJavaClassName(cview jvmClassName);
    bool isWideClassName(cview className);
}

#endif
