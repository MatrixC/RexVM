#ifndef NATIVE_JAVA_LANG_THREAD_HPP
#define NATIVE_JAVA_LANG_THREAD_HPP
#include "../config.hpp"
#include "../frame.hpp"

namespace RexVM::Native {

    void currentThread(Frame &frame);
    void isAlive(Frame &frame);
    void start0(Frame &frame);

}

#endif