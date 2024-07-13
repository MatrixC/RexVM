#ifndef CLASS_UTIL_HPP
#define CLASS_UTIL_HPP
#include "../config.hpp"

namespace RexVM {

    struct Oop;
    struct ClassLoader;
    struct ByteTypeArrayOop;
    struct Class;

    cstring getDescriptorClassName(const cstring &className);
    cstring getClassNameByFieldDescriptor(const cstring &descriptor);
    cstring getDescriptorByClass(Class *klass);
    cstring getJVMClassName(const cstring &javaClassName);
    cstring getJavaClassName(const cstring &jvmClassName);
}

#endif
