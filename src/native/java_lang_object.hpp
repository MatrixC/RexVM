#ifndef NATIVE_JAVA_LANG_OBJECT_HPP
#define NATIVE_JAVA_LANG_OBJECT_HPP
#include "../config.hpp"
#include "../frame.hpp"

namespace RexVM::Native {

    void getClass(Frame &frame);
    void hashCode(Frame &frame);
    void clone(Frame &frame);
    void notifyAll(Frame &frame);

}

#endif
