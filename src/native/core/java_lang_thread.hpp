#ifndef NATIVE_CORE_JAVA_LANG_THREAD_HPP
#define NATIVE_CORE_JAVA_LANG_THREAD_HPP
#include "../../config.hpp"
#include "../../frame.hpp"
#include "../../thread.hpp"
#include "../../oop.hpp"
#include "../../class.hpp"
#include "../../execute.hpp"
#include "../../memory.hpp"
#include <thread>

namespace RexVM::Native::Core {

    //public static native Thread currentThread();
    void currentThread(Frame &frame) {
        frame.returnRef(frame.thread.getThreadMirror());
    }

    //public static native void yield();
    void yield(Frame &frame) {
        std::this_thread::yield();
    }

    //public static native void sleep(long millis);
    void sleep(Frame &frame) {
        const auto millis = frame.getLocalI8(0);
        std::this_thread::sleep_for(std::chrono::milliseconds(millis));
    }

    //private native static Thread[] getThreads();
    //only use for getAllStackTraces, but getAllStackTraces is useless
    //return empty array
    void getThreads(Frame &frame) {
        frame.returnRef(nullptr);
    }

    //private native static StackTraceElement[][] dumpThreads(Thread[] threads);
    //only use for getAllStackTraces and getStackTrace, useless 
    void dumpThreads(Frame &frame) {
        frame.returnRef(nullptr);
    }

    //public final native boolean isAlive();
    void isAlive(Frame &frame) {
        frame.returnBoolean(frame.thread.status != ThreadStatusEnum::Terminated);
    }

    //public static native boolean holdsLock(Object obj);
    void holdsLock(Frame &frame) {
        frame.returnBoolean(true);
    }

    //private native void start0();
    void start0(Frame &frame) {
        const auto self = frame.getThisInstance(); //Thread Instance
        const auto threadClass = static_cast<InstanceClass *>(self->klass);
        if (threadClass->name == "java/lang/ref/Reference$ReferenceHandler") [[unlikely]] {
            // endless loop tryHandlePending
            return;
        }
        
        auto method = threadClass->getMethod("run", "()V", false);
        runStaticMethodOnNewThread(frame.vm, *method, nullptr, { Slot(self) });
    }

    //public native int countStackFrames();
    void countStackFrames(Frame &frame) {
        const auto vmThreadMirror = static_cast<ThreadOop *>(frame.getThisInstance());
        if (vmThreadMirror == nullptr) {
            frame.returnI4(0);
            return;
        }
        frame.returnI4(frame.level + 1);
    }

    //private native void interrupt0();
    void interrupt0(Frame &frame) {
    }

    //private native boolean isInterrupted(boolean ClearInterrupted);
    void isInterrupted(Frame &frame) {
    }

    //private native void setPriority0(int newPriority);
    void setPriority0(Frame &frame) {
    }

    //private native void setNativeName(String name);
    //useless
    void setNativeName(Frame &frame) {
    }

    //private native void stop0(Object o);
    //@Deprecated
    void stop0(Frame &frame) {
    }
    
    //private native void suspend0();
    //@Deprecated
    void suspend0(Frame &frame) {
    }

    //private native void resume0();
    //@Deprecated
    void resume0(Frame &frame) {
    }


}

#endif