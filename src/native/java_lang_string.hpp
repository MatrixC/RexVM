#ifndef NATIVE_JAVA_LANG_STRING_HPP
#define NATIVE_JAVA_LANG_STRING_HPP
#include "../config.hpp"
#include "../frame.hpp"

namespace RexVM::Native {

    void intern(Frame &frame);
}

#endif