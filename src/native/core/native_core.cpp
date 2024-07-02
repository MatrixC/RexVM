#include "../../config.hpp"
#include "../../basic_java_class.hpp"
#include "native_core.hpp"
#include "../native_manager.hpp"
#include "java_lang_thread.hpp"

namespace RexVM::Native::Core {

    void registerThreadCoreMethods(NativeManager &manager) {
        //static
        manager.regNativeMethod(JAVA_LANG_THREAD_NAME, "currentThread", "()Ljava/lang/Thread;", true, Native::Core::currentThread);
        manager.regNativeMethod(JAVA_LANG_THREAD_NAME, "yield", "()V", true, Native::Core::yield);
        manager.regNativeMethod(JAVA_LANG_THREAD_NAME, "sleep", "(J)V", true, Native::Core::sleep);
        manager.regNativeMethod(JAVA_LANG_THREAD_NAME, "getThreads", "()[Ljava/lang/Thread;", true, Native::Core::getThreads);
        manager.regNativeMethod(JAVA_LANG_THREAD_NAME, "dumpThreads", "()[[Ljava/lang/StackTraceElement;", true, Native::Core::dumpThreads);

        //normal
        manager.regNativeMethod(JAVA_LANG_THREAD_NAME, "isAlive", "()Z", false, Native::Core::isAlive);
        manager.regNativeMethod(JAVA_LANG_THREAD_NAME, "holdsLock", "()Z", false, Native::Core::holdsLock);
        manager.regNativeMethod(JAVA_LANG_THREAD_NAME, "start0", "()V", false, Native::Core::start0);
        manager.regNativeMethod(JAVA_LANG_THREAD_NAME, "countStackFrames", "()I", false, Native::Core::countStackFrames);
        manager.regNativeMethod(JAVA_LANG_THREAD_NAME, "interrupt0", "()V", false, Native::Core::interrupt0);
        manager.regNativeMethod(JAVA_LANG_THREAD_NAME, "isInterrupted", "(Z)Z", false, Native::Core::isInterrupted);
        manager.regNativeMethod(JAVA_LANG_THREAD_NAME, "setPriority0", "(I)V", false, Native::Core::setPriority0);
        manager.regNativeMethod(JAVA_LANG_THREAD_NAME, "setNativeName", "(Ljava/lang/String;)V", false, Native::Core::setNativeName);
        manager.regNativeMethod(JAVA_LANG_THREAD_NAME, "stop0", "(Ljava/lang/Object;)V", false, Native::Core::stop0);
        manager.regNativeMethod(JAVA_LANG_THREAD_NAME, "suspend0", "()V", false, Native::Core::suspend0);
        manager.regNativeMethod(JAVA_LANG_THREAD_NAME, "resume0", "()V", false, Native::Core::resume0);
    }
}