#ifndef NATIVE_JAVA_LANG_FLOAT_HPP
#define NATIVE_JAVA_LANG_FLOAT_HPP
#include "../config.hpp"
#include "../frame.hpp"

namespace RexVM::Native {

    void floatToRawIntBits(Frame &frame);
    void intBitsToFloat(Frame &frame);
    void doubleToRawLongBits(Frame &frame);
    void longBitsToDouble(Frame &frame);
}

#endif
