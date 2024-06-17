#ifndef NATIVE_JAVA_LANG_THROWABLE_HPP
#define NATIVE_JAVA_LANG_THROWABLE_HPP
#include "../config.hpp"
#include "../frame.hpp"

namespace RexVM::Native {

    void fillInStackTrace(Frame &frame);

}

#endif
