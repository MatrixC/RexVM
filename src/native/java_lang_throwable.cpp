#include "java_lang_throwable.hpp"

namespace RexVM::Native {

    void fillInStackTrace(Frame &frame) {
        //const auto thisRef = frame.getLocalVariable(0);
        //TODO
        //frame.ret(thisRef, SlotTypeEnum::REF);
        const auto thisRef = frame.getThis();
        frame.returnRef(thisRef);
    }
}