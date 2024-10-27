#ifndef NATIVE_CORE_JAVA_UTIL_CONCURRENT_ATOMIC_HPP
#define NATIVE_CORE_JAVA_UTIL_CONCURRENT_ATOMIC_HPP
#include "../../basic.hpp"
#include "../../vm.hpp"
#include "../../frame.hpp"
#include "../../thread.hpp"

namespace RexVM::Native::Core {

    void vmSupportsCS8(Frame &frame) {
        frame.returnBoolean(false);
    }
}

#endif