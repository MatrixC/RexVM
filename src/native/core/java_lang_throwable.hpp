#ifndef NATIVE_CORE_JAVA_LANG_THROWABLE_HPP
#define NATIVE_CORE_JAVA_LANG_THROWABLE_HPP
#include "../../config.hpp"
#include "../../vm.hpp"
#include "../../frame.hpp"
#include "../../thread.hpp"

namespace RexVM::Native::Core {

    void fillInStackTrace(Frame &frame) {
        //const auto thisRef = frame.getLocalVariable(0);
        //TODO
        //frame.ret(thisRef, SlotTypeEnum::REF);
        const auto thisRef = frame.getThis();
        frame.returnRef(thisRef);
    }
}

#endif