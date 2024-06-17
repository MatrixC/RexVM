#include "java_lang_thread.hpp"
#include "../runtime.hpp"

namespace RexVM::Native {

    void currentThread(Frame &frame) {
        const auto vmThreadRef = frame.thread.getThreadMirror();
        frame.returnRef(vmThreadRef);
    }

    void isAlive(Frame &frame) {
        frame.returnI4(0);
    }

}