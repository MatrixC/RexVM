#ifndef NATIVE_JAVA_LANG_CLASS_HPP
#define NATIVE_JAVA_LANG_CLASS_HPP
#include "../config.hpp"
#include "../frame.hpp"

namespace RexVM::Native {

    void getPrimitiveClass(Frame &frame);
    void desiredAssertionStatus0(Frame &frame);
    void getComponentType(Frame &frame);
    void forName0(Frame &frame);
    void getDeclaredFields0(Frame &frame);
    void isPrimitive(Frame &frame);
    void isInterface(Frame &frame);
    void isArray(Frame &frame);
    void isAssignableFrom(Frame &frame);

    void getDeclaredConstructors0(Frame &frame);
    void getModifiers(Frame &frame);
    void getSuperclass(Frame &frame);

}

#endif
