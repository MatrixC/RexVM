#ifndef NATIVE_CORE_JAVA_LANG_SYSTEM_HPP
#define NATIVE_CORE_JAVA_LANG_SYSTEM_HPP

#include "../../config.hpp"
#include "../../vm.hpp"
#include "../../frame.hpp"
#include "../../thread.hpp"
#include "../../oop.hpp"
#include "../../class.hpp"
#include "../../execute.hpp"
#include "../../memory.hpp"
#include <thread>
#include <chrono>


namespace RexVM::Native::Core {

    //java/lang/System#nanoTime:()J
    void nanoTime(Frame &frame) {
        const auto now = std::chrono::high_resolution_clock::now();
        const auto duration = now.time_since_epoch();
        const auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
        frame.returnI8(nanos);
    }

    //static native int identityHashCode(Object x);
    void identityHashCode(Frame &frame) {
        frame.returnI4(static_cast<i4>(std::bit_cast<u8>(frame.getLocalRef(0))));
    }
}

#endif