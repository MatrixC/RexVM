#include "self_native.hpp"
#include "../utils/format.hpp"
#include "../utils/class_utils.hpp"

namespace RexVM::Native {

    void selfPrintln(Frame &frame) {
        auto str = frame.getLocalRef(0);
        println("{}", getStringNativeValue(static_cast<Oop *>(str)));
    }

    void selfPrintlnInt(Frame &frame) {
        auto val = frame.getLocalI4(0);
        println("{}", val);
    }
}