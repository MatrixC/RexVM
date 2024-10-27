#ifndef NATIVE_CORE_JAVA_LANG_FLOAT_HPP
#define NATIVE_CORE_JAVA_LANG_FLOAT_HPP
#include <cmath>
#include "../../basic.hpp"
#include "../../vm.hpp"
#include "../../frame.hpp"
#include "../../thread.hpp"

namespace RexVM::Native::Core {

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

    void _sin(Frame &frame) {
        const auto a = frame.getLocalF8(0);
        frame.returnF8(std::sin(a));
    }

    void _cos(Frame &frame) {
        const auto a = frame.getLocalF8(0);
        frame.returnF8(std::cos(a));
    }

    void _tan(Frame &frame) {
        const auto a = frame.getLocalF8(0);
        frame.returnF8(std::tan(a));
    }

    void _asin(Frame &frame) {
        const auto a = frame.getLocalF8(0);
        frame.returnF8(std::asin(a));
    }

    void _acos(Frame &frame) {
        const auto a = frame.getLocalF8(0);
        frame.returnF8(std::acos(a));
    }

    void _atan(Frame &frame) {
        const auto a = frame.getLocalF8(0);
        frame.returnF8(std::atan(a));
    }

    void _atan2(Frame &frame) {
        const auto a = frame.getLocalF8(0);
        const auto b = frame.getLocalF8(2);
        frame.returnF8(std::atan2(a, b));
    }

    void _exp(Frame &frame) {
        const auto a = frame.getLocalF8(0);
        frame.returnF8(std::exp(a));
    }

    void _log(Frame &frame) {
        const auto a = frame.getLocalF8(0);
        frame.returnF8(std::log(a));
    }

    void _log10(Frame &frame) {
        const auto a = frame.getLocalF8(0);
        frame.returnF8(std::log10(a));
    }

    void _sqrt(Frame &frame) {
        const auto a = frame.getLocalF8(0);
        frame.returnF8(std::sqrt(a));
    }

    void _pow(Frame &frame) {
        const auto a = frame.getLocalF8(0);
        const auto b = frame.getLocalF8(2);
        frame.returnF8(std::pow(a, b));
    }

    void _cbrt(Frame &frame) {
        const auto a = frame.getLocalF8(0);
        frame.returnF8(std::cbrt(a));
    }

    void _ieeeremainder(Frame &frame) {
        const auto a = frame.getLocalF8(0);
        const auto b = frame.getLocalF8(2);
        frame.returnF8(std::remainder(a, b));
    }

    void _sinh(Frame &frame) {
        const auto a = frame.getLocalF8(0);
        frame.returnF8(std::sinh(a));
    }

    void _cosh(Frame &frame) {
        const auto a = frame.getLocalF8(0);
        frame.returnF8(std::cosh(a));
    }

    void _tanh(Frame &frame) {
        const auto a = frame.getLocalF8(0);
        frame.returnF8(std::tanh(a));
    }

    void _hypot(Frame &frame) {
        const auto a = frame.getLocalF8(0);
        const auto b = frame.getLocalF8(2);
        frame.returnF8(std::hypot(a, b));
    }

    void _expm1(Frame &frame) {
        const auto a = frame.getLocalF8(0);
        frame.returnF8(std::expm1(a));
    }

    void _log1p(Frame &frame) {
        const auto a = frame.getLocalF8(0);
        frame.returnF8(std::log1p(a));
    }

}

#endif