#include "java_lang_thread.hpp"
#include "../utils/format.hpp"
#include "../class.hpp"
#include "../oop.hpp"
#include "../thread.hpp"
#include "../execute.hpp"

namespace RexVM::Native {

    void currentThread(Frame &frame) {
        const auto vmThreadRef = frame.thread.getThreadMirror();
        frame.returnRef(vmThreadRef);
    }

    void isAlive(Frame &frame) {
        frame.returnI4(0);
    }

    void start0(Frame &frame) {
        const auto self = static_cast<InstanceOop *>(frame.getThis());
        const auto threadClass = static_cast<InstanceClass *>(self->klass);
        auto method = threadClass->getMethod("run", "()V", false);
        runStaticMethodOnNewThread(frame.vm, *method, {});
        println("start 0: {}, {}", threadClass->name, method->name);
        (void) method;
    }

}