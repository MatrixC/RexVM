#ifndef SELF_NATIVE_HPP
#define SELF_NATIVE_HPP

#include "../config.hpp"
#include "../frame.hpp"

namespace RexVM::Native {

    void selfPrintln(Frame &frame);
    void selfPrintlnInt(Frame &frame);
}

#endif