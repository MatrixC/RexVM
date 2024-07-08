#include "../../config.hpp"
#include "../../basic_java_class.hpp"
#include "native_core.hpp"
#include "../native_manager.hpp"

#include "java_lang_object.hpp"
#include "java_lang_thread.hpp"
#include "java_lang_class.hpp"
#include "java_lang_runtime.hpp"
#include "java_lang_string.hpp"
#include "java_lang_reflect_array.hpp"

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

    void registerStringCoreMethods(NativeManager &manager) {
        manager.regNativeMethod(JAVA_LANG_STRING_NAME, "intern", "()Ljava/lang/String;", false, Native::Core::intern);
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
        manager.regNativeMethod(JAVA_LANG_CLASS_NAME, "getName0", "()Ljava/lang/String;", false, Native::Core::getName0);
        manager.regNativeMethod(JAVA_LANG_CLASS_NAME, "forName0", "(Ljava/lang/String;ZLjava/lang/ClassLoader;Ljava/lang/Class;)Ljava/lang/Class;", true, Native::Core::forName0);
        manager.regNativeMethod(JAVA_LANG_CLASS_NAME, "desiredAssertionStatus0", "(Ljava/lang/Class;)Z", true, Native::Core::desiredAssertionStatus0);
        manager.regNativeMethod(JAVA_LANG_CLASS_NAME, "getPrimitiveClass", "(Ljava/lang/String;)Ljava/lang/Class;", true,  Native::Core::getPrimitiveClass);
        manager.regNativeMethod(JAVA_LANG_CLASS_NAME, "isPrimitive", "()Z", false, Native::Core::isPrimitive);
        manager.regNativeMethod(JAVA_LANG_CLASS_NAME, "isInterface", "()Z", false, Native::Core::isInterface);
        manager.regNativeMethod(JAVA_LANG_CLASS_NAME, "isArray", "()Z", false, Native::Core::isArray);
        manager.regNativeMethod(JAVA_LANG_CLASS_NAME, "isAssignableFrom", "(Ljava/lang/Class;)Z", true, Native::Core::isAssignableFrom);
        manager.regNativeMethod(JAVA_LANG_CLASS_NAME, "getModifiers", "()I", true, Native::Core::getModifiers);
        manager.regNativeMethod(JAVA_LANG_CLASS_NAME, "getSuperclass", "()Ljava/lang/Class;", true, Native::Core::getSuperclass);
        manager.regNativeMethod(JAVA_LANG_CLASS_NAME, "getComponentType", "()Ljava/lang/Class;", true, Native::Core::getComponentType);
        manager.regNativeMethod(JAVA_LANG_CLASS_NAME, "getEnclosingMethod0", "()[Ljava/lang/Object;", false, Native::Core::getEnclosingMethod0);    
        manager.regNativeMethod(JAVA_LANG_CLASS_NAME, "getDeclaringClass0", "()Ljava/lang/Class;", false, Native::Core::getDeclaringClass0);
        manager.regNativeMethod(JAVA_LANG_CLASS_NAME, "getDeclaredFields0", "(Z)[Ljava/lang/reflect/Field;", false, Native::Core::getDeclaredFields0);
        manager.regNativeMethod(JAVA_LANG_CLASS_NAME, "getDeclaredConstructors0", "(Z)[Ljava/lang/reflect/Constructor;", false, Native::Core::getDeclaredConstructors0);
        manager.regNativeMethod(JAVA_LANG_CLASS_NAME, "getDeclaredMethods0", "(Z)[Ljava/lang/reflect/Method;", false, Native::Core::getDeclaredMethods0);
        manager.regNativeMethod(JAVA_LANG_CLASS_NAME, "getInterfaces0", "()[Ljava/lang/Class;", false, Native::Core::getInterfaces0);
        manager.regNativeMethod(JAVA_LANG_CLASS_NAME, "isInstance", "(Ljava/lang/Object;)Z", false, Native::Core::isInstance);
        manager.regNativeMethod(JAVA_LANG_CLASS_NAME, "getSigners", "()[Ljava/lang/Object;", false, Native::Core::getSigners);
        manager.regNativeMethod(JAVA_LANG_CLASS_NAME, "setSigners", "([Ljava/lang/Object;)V", false, Native::Core::setSigners);
        manager.regNativeMethod(JAVA_LANG_CLASS_NAME, "getProtectionDomain0", "()Ljava/security/ProtectionDomain;", false, Native::Core::getProtectionDomain0);
        manager.regNativeMethod(JAVA_LANG_CLASS_NAME, "getGenericSignature0", "()Ljava/lang/String;", false, Native::Core::getGenericSignature0);
        manager.regNativeMethod(JAVA_LANG_CLASS_NAME, "getRawAnnotations", "()[B", false, Native::Core::getRawAnnotations);
        manager.regNativeMethod(JAVA_LANG_CLASS_NAME, "getRawTypeAnnotations", "()[B", false, Native::Core::getRawTypeAnnotations);
        manager.regNativeMethod(JAVA_LANG_CLASS_NAME, "getConstantPool", "()Lsun/reflect/ConstantPool;", false, Native::Core::getConstantPool);
        manager.regNativeMethod(JAVA_LANG_CLASS_NAME, "getDeclaredClasses0", "()[Ljava/lang/Class;", false, Native::Core::getDeclaredClasses0);


        manager.regNativeMethod("sun/reflect/ConstantPool", "getClassAt0", "(Ljava/lang/Object;I)Ljava/lang/Class;", false, Native::Core::getClassAt0);
        manager.regNativeMethod("sun/reflect/ConstantPool", "getIntAt0", "(Ljava/lang/Object;I)I", false, Native::Core::getIntAt0);
        manager.regNativeMethod("sun/reflect/ConstantPool", "getLongAt0", "(Ljava/lang/Object;I)J", false, Native::Core::getLongAt0);
        manager.regNativeMethod("sun/reflect/ConstantPool", "getFloatAt0", "(Ljava/lang/Object;I)F", false, Native::Core::getFloatAt0);
        manager.regNativeMethod("sun/reflect/ConstantPool", "getDoubleAt0", "(Ljava/lang/Object;I)D", false, Native::Core::getDoubleAt0);
        manager.regNativeMethod("sun/reflect/ConstantPool", "getUTF8At0", "(Ljava/lang/Object;I)Ljava/lang/String;", false, Native::Core::getUTF8At0);
    }

    void registerRuntimeCoreMethods(NativeManager &manager) {
        manager.regNativeMethod(JAVA_LANG_RUNTIME_NAME, "availableProcessors", "()I", true, Native::Core::availableProcessors);
    }

    constexpr auto JAVA_LANG_INVOKE_METHOD_HANDLE_NATIVS_NAME = "java/lang/invoke/MethodHandleNatives";
    void registerInvokeCoreMethods(NativeManager &manager) {
        manager.regNativeMethod(JAVA_LANG_INVOKE_METHOD_HANDLE_NATIVS_NAME, "getConstant", "(I)I", true, Native::Core::getConstant);
        manager.regNativeMethod(JAVA_LANG_INVOKE_METHOD_HANDLE_NATIVS_NAME, "resolve", "(Ljava/lang/invoke/MemberName;Ljava/lang/Class;)Ljava/lang/invoke/MemberName;", true, Native::Core::getConstant);
    }

    constexpr auto JAVA_LANG_REFLECT_ARRAY_NAME = "java/lang/reflect/Array";
    void registerReflectCoreMethods(NativeManager &manager) {
        manager.regNativeMethod(JAVA_LANG_REFLECT_ARRAY_NAME, "newArray", "(Ljava/lang/Class;I)Ljava/lang/Object;", true, Native::Core::newArray);
    }

    void registerCoreMethods(NativeManager &manager) {
        registerObjectCoreMethods(manager);
        registerStringCoreMethods(manager);
        registerThreadCoreMethods(manager);
        registerClassCoreMethods(manager);
        registerRuntimeCoreMethods(manager);
        registerInvokeCoreMethods(manager);
        registerReflectCoreMethods(manager);

    }

}