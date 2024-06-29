#ifndef CLASS_UTIL_HPP
#define CLASS_UTIL_HPP
#include "../config.hpp"

namespace RexVM {

    struct Oop;
    struct ClassLoader;
    struct ByteTypeArrayOop;

    cstring getDescriptorClassName(const cstring &className);

    cstring getClassNameByFieldDescriptor(const cstring &descriptor);
}

#endif
