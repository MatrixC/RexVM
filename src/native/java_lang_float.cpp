#include "java_lang_float.hpp"
#include <bit>

namespace RexVM::Native {

    void floatToRawIntBits(Frame &frame) {
        const auto value = frame.getLocalF4(0);
        frame.returnI4(std::bit_cast<i4>(value));
    }

    void intBitsToFloat(Frame &frame) {
        const auto value = frame.getLocalI4(0);
        frame.returnF4(std::bit_cast<f4>(value));
    }

    void doubleToRawLongBits(Frame &frame) {
        const auto value = frame.getLocalF8(0);
        frame.returnI8(std::bit_cast<i8>(value));
    }

    void longBitsToDouble(Frame &frame) {
        const auto value = frame.getLocalI8(0);
        frame.returnF8(std::bit_cast<f8>(value));
    }
}