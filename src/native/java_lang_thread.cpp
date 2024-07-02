#include "java_lang_thread.hpp"
#include "../utils/format.hpp"
#include "../class.hpp"
#include "../oop.hpp"
#include "../thread.hpp"
#include "../execute.hpp"

namespace RexVM::Native {

    void currentThread(Frame &frame) {
        const auto vmThreadRef = &frame.thread;
        frame.returnRef(vmThreadRef);
    }

    void isAlive(Frame &frame) {
        frame.returnI4(0);
    }

    void start0(Frame &frame) {
        const auto self = static_cast<InstanceOop *>(frame.getThis());
        const auto threadClass = static_cast<InstanceClass *>(self->klass);
        if (threadClass->name == "java/lang/ref/Reference$ReferenceHandler") [[unlikely]] {
            // endless loop tryHandlePending
            return;
        }
        
        auto method = threadClass->getMethod("run", "()V", false);
        runStaticMethodOnNewThread(frame.vm, *method, std::vector<Slot>{ Slot(self) });
    }

}