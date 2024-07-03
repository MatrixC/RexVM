#include "../../config.hpp"
#include "../../basic_java_class.hpp"
#include "native_core.hpp"
#include "../native_manager.hpp"

#include "java_lang_object.hpp"
#include "java_lang_thread.hpp"
#include "java_lang_class.hpp"
#include "java_lang_runtime.hpp"

#include "java_lang_invoke_method_handle_natives.hpp"

namespace RexVM::Native::Core {

    void registerObjectCoreMethods(NativeManager &manager) {
        manager.regNativeMethod(JAVA_LANG_OBJECT_NAME, "getClass", "()Ljava/lang/Class;", false, Native::Core::getClass);
        manager.regNativeMethod(JAVA_LANG_OBJECT_NAME, "hashCode", "()I", false, Native::Core::hashCode);
        manager.regNativeMethod(JAVA_LANG_OBJECT_NAME, "clone", "()Ljava/lang/Object;", false, Native::Core::clone);
        manager.regNativeMethod(JAVA_LANG_OBJECT_NAME, "wait", "(J)V", false, Native::Core::wait);
        manager.regNativeMethod(JAVA_LANG_OBJECT_NAME, "notify", "()V", false, Native::Core::notify);
        manager.regNativeMethod(JAVA_LANG_OBJECT_NAME, "notifyAll", "()V", false, Native::Core::notifyAll);
    }

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

    void registerClassCoreMethods(NativeManager &manager) {
        manager.regNativeMethod(JAVA_LANG_CLASS_NAME, "getName0", "()Ljava/lang/String;", true, Native::Core::getName0);
        manager.regNativeMethod(JAVA_LANG_CLASS_NAME, "getEnclosingMethod0", "()[Ljava/lang/Object;", false, Native::Core::getEnclosingMethod0);    
        manager.regNativeMethod(JAVA_LANG_CLASS_NAME, "getDeclaringClass0", "()Ljava/lang/Class;", false, Native::Core::getDeclaringClass0);
    }

    void registerRuntimeCoreMethods(NativeManager &manager) {
        manager.regNativeMethod(JAVA_LANG_RUNTIME_NAME, "availableProcessors", "()I", true, Native::Core::availableProcessors);
    }

    constexpr auto JAVA_LANG_INVOKE_METHOD_HANDLE_NATIVS = "java/lang/invoke/MethodHandleNatives";
    void registerInvokeCoreMethods(NativeManager &manager) {
        manager.regNativeMethod(JAVA_LANG_INVOKE_METHOD_HANDLE_NATIVS, "getConstant", "(I)I", true, Native::Core::getConstant);
        manager.regNativeMethod(JAVA_LANG_INVOKE_METHOD_HANDLE_NATIVS, "resolve", "(Ljava/lang/invoke/MemberName;Ljava/lang/Class;)Ljava/lang/invoke/MemberName;", true, Native::Core::getConstant);
    }

}