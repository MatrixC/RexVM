#ifndef NATIVE_CORE_JAVA_LANG_SHUTDOWN_HPP
#define NATIVE_CORE_JAVA_LANG_SHUTDOWN_HPP
#include "../../config.hpp"
#include "../../vm.hpp"
#include "../../frame.hpp"

namespace RexVM::Native::Core {

    //static native void beforeHalt();
    void beforeHalt(Frame &frame) {
    }

    //static native void halt0(int status);
    void halt0(Frame &frame) {
        std::exit(frame.getLocalI4(0));
    }

}

#endif