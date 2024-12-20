#include "../../basic.hpp"
#include "../../basic_java_class.hpp"
#include "native_core.hpp"

#include "java_lang_object.hpp"
#include "java_lang_thread.hpp"
#include "java_lang_system.hpp"
#include "java_lang_class.hpp"
#include "java_lang_class_loader.hpp"
#include "java_lang_runtime.hpp"
#include "java_lang_string.hpp"
#include "java_lang_reflect_array.hpp"
#include "java_lang_float.hpp"
#include "java_lang_throwable.hpp"
#include "java_io_file_input_output_stream.hpp"
#include "java_io_misc.hpp"
#include "java_lang_shutdown.hpp"
#include "java_util_concurrent_atomic.hpp"
#include "java_util_time_zone.hpp"
#include "java_lang_package.hpp"
#include "unsafe.hpp"
#include "misc.hpp"
#include "define_class.hpp"

#include "java_lang_invoke_method_handle_natives.hpp"

namespace RexVM::Native::Core {
    constexpr auto UNSAFE_CLASS_NAME = "sun/misc/Unsafe";
    constexpr auto JAVA_LANG_INVOKE_METHOD_HANDLE_NATIVES_NAME = "java/lang/invoke/MethodHandleNatives";
    constexpr auto JAVA_LANG_INVOKE_METHOD_HANDLE_NAME = "java/lang/invoke/MethodHandle";
    constexpr auto JAVA_LANG_REFLECT_ARRAY_NAME = "java/lang/reflect/Array";
    constexpr auto JAVA_LANG_REFLECT_PROXY_NAME = "java/lang/reflect/Proxy";
    constexpr auto JAVA_LANG_REFLECT_FIELD_NAME = "java/lang/reflect/Field";
    constexpr auto JAVA_LANG_REFLECT_EXECUTEABLE_NAME = "java/lang/reflect/Executable";
    constexpr auto JAVA_LANG_PACKAGE_NAME = "java/lang/Package";
    constexpr auto JAVA_LANG_STRICT_MATH_NAME = "java/lang/StrictMath";
    constexpr auto JAVA_UTIL_CONCURRENT_ATOMIC_NAME = "java/util/concurrent/atomic/AtomicLong";
    constexpr auto JAVA_UTIL_TIME_ZONE_NAME = "java/util/TimeZone";
    constexpr auto JAVA_SECURITY_ACCESS_CONTROLLER_NAME = "java/security/AccessController";
    constexpr auto JAVA_IO_FILE_INPUT_STREAM_NAME = "java/io/FileInputStream";
    constexpr auto JAVA_IO_FILE_OUTPUT_STREAM_NAME = "java/io/FileOutputStream";
    constexpr auto JAVA_IO_UNIX_FILE_SYSTEM_NAME = "java/io/UnixFileSystem";
    constexpr auto JAVA_IO_OBJECT_STREAM_NAME = "java/io/ObjectStreamClass";
    constexpr auto SUN_REFLECT_NATIVE_METHOD_ACCESSOR_IMPL_NAME = "sun/reflect/NativeMethodAccessorImpl";
    constexpr auto SUN_REFLECT_CONSTANT_POOL_NAME = "sun/reflect/ConstantPool";
    constexpr auto SUN_REFLECT_REFLECTION_NAME = "sun/reflect/Reflection";

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

    void registerSystemCoreMethods(NativeManager &manager) {
        manager.regNativeMethod(JAVA_LANG_SYSTEM_NAME, "currentTimeMillis", "()J", false, Native::Core::currentTimeMillis);
        manager.regNativeMethod(JAVA_LANG_SYSTEM_NAME, "nanoTime", "()J", true, Native::Core::nanoTime);
        manager.regNativeMethod(JAVA_LANG_SYSTEM_NAME, "identityHashCode", "(Ljava/lang/Object;)I", true, Native::Core::identityHashCode);

        manager.regNativeMethod(JAVA_LANG_SYSTEM_NAME, "arraycopy", "(Ljava/lang/Object;ILjava/lang/Object;II)V", false, Native::Core::arraycopy);
        manager.regNativeMethod(JAVA_LANG_SYSTEM_NAME, "initProperties", "(Ljava/util/Properties;)Ljava/util/Properties;", false, Native::Core::initProperties);
        manager.regNativeMethod(JAVA_LANG_SYSTEM_NAME, "mapLibraryName", "(Ljava/lang/String;)Ljava/lang/String;", false, Native::Core::mapLibraryName);

        manager.regNativeMethod(JAVA_LANG_SYSTEM_NAME, "setIn0", "(Ljava/io/InputStream;)V", false, Native::Core::setIn0);
        manager.regNativeMethod(JAVA_LANG_SYSTEM_NAME, "setOut0", "(Ljava/io/PrintStream;)V", false, Native::Core::setOut0);
        manager.regNativeMethod(JAVA_LANG_SYSTEM_NAME, "setErr0", "(Ljava/io/PrintStream;)V", false, Native::Core::setErr0);

        manager.regNativeMethod(JAVA_SECURITY_ACCESS_CONTROLLER_NAME, "doPrivileged", "(Ljava/security/PrivilegedAction;)Ljava/lang/Object;", false, Native::Core::doPrivileged);
        manager.regNativeMethod(JAVA_SECURITY_ACCESS_CONTROLLER_NAME, "doPrivileged", "(Ljava/security/PrivilegedAction;Ljava/security/AccessControlContext;)Ljava/lang/Object;", false, Native::Core::doPrivileged);
        manager.regNativeMethod(JAVA_SECURITY_ACCESS_CONTROLLER_NAME, "doPrivileged", "(Ljava/security/PrivilegedExceptionAction;)Ljava/lang/Object;", false, Native::Core::doPrivileged);
        manager.regNativeMethod(JAVA_SECURITY_ACCESS_CONTROLLER_NAME, "doPrivileged", "(Ljava/security/PrivilegedExceptionAction;Ljava/security/AccessControlContext;)Ljava/lang/Object;", false, Native::Core::doPrivileged);
        manager.regNativeMethod(JAVA_SECURITY_ACCESS_CONTROLLER_NAME, "doPrivileged", "(Ljava/security/PrivilegedAction;)Ljava/lang/Object;", false, Native::Core::doPrivileged);
        manager.regNativeMethod(JAVA_SECURITY_ACCESS_CONTROLLER_NAME, "getStackAccessControlContext", "()Ljava/security/AccessControlContext;", false, Native::Core::getStackAccessControlContext);
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

        manager.regNativeMethod(JAVA_LANG_REFLECT_FIELD_NAME, "getTypeAnnotationBytes0", "()[B", false, Native::Core::fieldGetTypeAnnotationBytes0);
        manager.regNativeMethod(JAVA_LANG_REFLECT_EXECUTEABLE_NAME, "getParameters0", "()[Ljava/lang/reflect/Parameter;", false, Native::Core::getParameters0);

        manager.regNativeMethod(SUN_REFLECT_CONSTANT_POOL_NAME, "getClassAt0", "(Ljava/lang/Object;I)Ljava/lang/Class;", false, Native::Core::getClassAt0);
        manager.regNativeMethod(SUN_REFLECT_CONSTANT_POOL_NAME, "getIntAt0", "(Ljava/lang/Object;I)I", false, Native::Core::getIntAt0);
        manager.regNativeMethod(SUN_REFLECT_CONSTANT_POOL_NAME, "getLongAt0", "(Ljava/lang/Object;I)J", false, Native::Core::getLongAt0);
        manager.regNativeMethod(SUN_REFLECT_CONSTANT_POOL_NAME, "getFloatAt0", "(Ljava/lang/Object;I)F", false, Native::Core::getFloatAt0);
        manager.regNativeMethod(SUN_REFLECT_CONSTANT_POOL_NAME, "getDoubleAt0", "(Ljava/lang/Object;I)D", false, Native::Core::getDoubleAt0);
        manager.regNativeMethod(SUN_REFLECT_CONSTANT_POOL_NAME, "getUTF8At0", "(Ljava/lang/Object;I)Ljava/lang/String;", false, Native::Core::getUTF8At0);

        manager.regNativeMethod(SUN_REFLECT_REFLECTION_NAME, "getCallerClass", "()Ljava/lang/Class;", false, Native::Core::getCallerClass);
        manager.regNativeMethod(SUN_REFLECT_REFLECTION_NAME, "getClassAccessFlags", "(Ljava/lang/Class;)I", false, Native::Core::getClassAccessFlags);

        manager.regNativeMethod(SUN_REFLECT_NATIVE_METHOD_ACCESSOR_IMPL_NAME, "invoke0", "(Ljava/lang/reflect/Method;Ljava/lang/Object;[Ljava/lang/Object;)Ljava/lang/Object;", false, Native::Core::invoke0);
    }

    void registerClassLoaderCoreMethods(NativeManager &manager) {
        manager.regNativeMethod(JAVA_LANG_CLASS_LOADER_NAME, "defineClass0", "(Ljava/lang/String;[BIILjava/security/ProtectionDomain;)Ljava/lang/Class;", false, Native::Core::classLoaderDefineClass0);
        manager.regNativeMethod(JAVA_LANG_CLASS_LOADER_NAME, "defineClass1", "(Ljava/lang/String;[BIILjava/security/ProtectionDomain;Ljava/lang/String;)Ljava/lang/Class;", false, Native::Core::classLoaderDefineClass0);
        manager.regNativeMethod(JAVA_LANG_CLASS_LOADER_NAME, "defineClass2", "(Ljava/lang/String;Ljava/nio/ByteBuffer;IILjava/security/ProtectionDomain;Ljava/lang/String;)Ljava/lang/Class;", false, Native::Core::classLoaderDefineClass2);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "defineClass", "(Ljava/lang/String;[BIILjava/lang/ClassLoader;Ljava/security/ProtectionDomain;)Ljava/lang/Class;", false, Native::Core::unsafeDefineClass);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "defineAnonymousClass", "(Ljava/lang/Class;[B[Ljava/lang/Object;)Ljava/lang/Class;", false, Native::Core::unsafeDefineAnonymousClass);
        manager.regNativeMethod(JAVA_LANG_REFLECT_PROXY_NAME, "defineClass0", "(Ljava/lang/ClassLoader;Ljava/lang/String;[BII)Ljava/lang/Class;", false, Native::Core::proxyDefineClass0);

        manager.regNativeMethod(JAVA_LANG_CLASS_LOADER_NAME, "resolveClass0", "(Ljava/lang/Class;)V", false, Native::Core::resolveClass0);
        manager.regNativeMethod(JAVA_LANG_CLASS_LOADER_NAME, "findBootstrapClass", "(Ljava/lang/String;)Ljava/lang/Class;", false, Native::Core::findBootstrapClass);
        manager.regNativeMethod(JAVA_LANG_CLASS_LOADER_NAME, "findLoadedClass0", "(Ljava/lang/String;)Ljava/lang/Class;", false, Native::Core::findLoadedClass0);
    }

    void registerRuntimeCoreMethods(NativeManager &manager) {
        manager.regNativeMethod(JAVA_LANG_RUNTIME_NAME, "availableProcessors", "()I", true, Native::Core::availableProcessors);
        manager.regNativeMethod(JAVA_LANG_RUNTIME_NAME, "gc", "()V", true, Native::Core::_gc);
    }

    void registerInvokeCoreMethods(NativeManager &manager) {
        manager.regNativeMethod(JAVA_LANG_INVOKE_METHOD_HANDLE_NATIVES_NAME, "getConstant", "(I)I", true, Native::Core::methodHandlerGetConstant);
        manager.regNativeMethod(JAVA_LANG_INVOKE_METHOD_HANDLE_NATIVES_NAME, "resolve", "(Ljava/lang/invoke/MemberName;Ljava/lang/Class;)Ljava/lang/invoke/MemberName;", true, Native::Core::methodHandlerResolve);
        manager.regNativeMethod(JAVA_LANG_INVOKE_METHOD_HANDLE_NATIVES_NAME, "init", "(Ljava/lang/invoke/MemberName;Ljava/lang/Object;)V", true, Native::Core::methodHandlerInit);
        manager.regNativeMethod(JAVA_LANG_INVOKE_METHOD_HANDLE_NATIVES_NAME, "objectFieldOffset", "(Ljava/lang/invoke/MemberName;)J", true, Native::Core::methodHandleObjectFieldOffset);
        manager.regNativeMethod(JAVA_LANG_INVOKE_METHOD_HANDLE_NATIVES_NAME, "getMembers", "(Ljava/lang/Class;Ljava/lang/String;Ljava/lang/String;ILjava/lang/Class;I[Ljava/lang/invoke/MemberName;)I", true, Native::Core::methodHandleGetMembers);


        manager.regNativeMethod(JAVA_LANG_INVOKE_METHOD_HANDLE_NAME, "invoke", "([Ljava/lang/Object;)Ljava/lang/Object;", true, Native::Core::methodHandleInvoke);
        manager.regNativeMethod(JAVA_LANG_INVOKE_METHOD_HANDLE_NAME, "invokeBasic", "([Ljava/lang/Object;)Ljava/lang/Object;", true, Native::Core::methodHandleInvoke);
        manager.regNativeMethod(JAVA_LANG_INVOKE_METHOD_HANDLE_NAME, "invokeExact", "([Ljava/lang/Object;)Ljava/lang/Object;", true, Native::Core::methodHandleInvoke);
    }

    void registerReflectCoreMethods(NativeManager &manager) {
        manager.regNativeMethod(JAVA_LANG_REFLECT_ARRAY_NAME, "newArray", "(Ljava/lang/Class;I)Ljava/lang/Object;", true, Native::Core::newArray);
    }
    
    void registerUnsafeCoreMethods(NativeManager &manager) {
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "ensureClassInitialized", "(Ljava/lang/Class;)V", false, Native::Core::ensureClassInitialized);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "shouldBeInitialized", "(Ljava/lang/Class;)Z", false, Native::Core::shouldBeInitialized);
        
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "arrayBaseOffset", "(Ljava/lang/Class;)I", false, Native::Core::arrayBaseOffset);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "arrayIndexScale", "(Ljava/lang/Class;)I", false, Native::Core::arrayIndexScale);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "addressSize", "()I", false, Native::Core::addressSize);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "objectFieldOffset", "(Ljava/lang/reflect/Field;)J", false, Native::Core::objectFieldOffset);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "staticFieldOffset", "(Ljava/lang/reflect/Field;)J", false, Native::Core::staticFieldOffset);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "staticFieldBase", "(Ljava/lang/reflect/Field;)Ljava/lang/Object;", false, Native::Core::staticFieldBase);

        manager.regNativeMethod(UNSAFE_CLASS_NAME, "compareAndSwapObject", "(Ljava/lang/Object;JLjava/lang/Object;Ljava/lang/Object;)Z", false, Native::Core::compareAndSwapObject);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "compareAndSwapInt", "(Ljava/lang/Object;JII)Z", false, Native::Core::compareAndSwapInt);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "compareAndSwapLong", "(Ljava/lang/Object;JJJ)Z", false, Native::Core::compareAndSwapLong);

        manager.regNativeMethod(UNSAFE_CLASS_NAME, "getObjectVolatile", "(Ljava/lang/Object;J)Ljava/lang/Object;", false, Native::Core::getObjectVolatile);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "putObjectVolatile", "(Ljava/lang/Object;JLjava/lang/Object;)V", false, Native::Core::putObjectVolatile);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "getBooleanVolatile", "(Ljava/lang/Object;J)Z", false, Native::Core::getVolatileI4);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "putBooleanVolatile", "(Ljava/lang/Object;JZ)V", false, Native::Core::putVolatileI4);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "getByteVolatile", "(Ljava/lang/Object;J)B", false, Native::Core::getVolatileI4);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "putByteVolatile", "(Ljava/lang/Object;JB)V", false, Native::Core::putVolatileI4);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "getShortVolatile", "(Ljava/lang/Object;J)S", false, Native::Core::getVolatileI4);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "putShortVolatile", "(Ljava/lang/Object;JS)V", false, Native::Core::putVolatileI4);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "getCharVolatile", "(Ljava/lang/Object;J)C", false, Native::Core::getVolatileI4);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "putCharVolatile", "(Ljava/lang/Object;JC)V", false, Native::Core::putVolatileI4);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "getIntVolatile", "(Ljava/lang/Object;J)I", false, Native::Core::getVolatileI4);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "putIntVolatile", "(Ljava/lang/Object;JI)V", false, Native::Core::putVolatileI4);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "getLongVolatile", "(Ljava/lang/Object;J)J", false, Native::Core::getLongVolatile);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "putLongVolatile", "(Ljava/lang/Object;JJ)V", false, Native::Core::putLongVolatile);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "getFloatVolatile", "(Ljava/lang/Object;J)F", false, Native::Core::getFloatVolatile);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "putFloatVolatile", "(Ljava/lang/Object;JF)V", false, Native::Core::putFloatVolatile);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "getDoubleVolatile", "(Ljava/lang/Object;J)D", false, Native::Core::getDoubleVolatile);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "putDoubleVolatile", "(Ljava/lang/Object;JD)V", false, Native::Core::putDoubleVolatile);

        manager.regNativeMethod(UNSAFE_CLASS_NAME, "getObject", "(Ljava/lang/Object;J)Ljava/lang/Object;", false, Native::Core::getObject);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "putObject", "(Ljava/lang/Object;JLjava/lang/Object;)V", false, Native::Core::putObject);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "getBoolean", "(Ljava/lang/Object;J)Z", false, Native::Core::getI4);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "putBoolean", "(Ljava/lang/Object;JZ)V", false, Native::Core::putI4);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "getByte", "(Ljava/lang/Object;J)B", false, Native::Core::getI4);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "putByte", "(Ljava/lang/Object;JB)V", false, Native::Core::putI4);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "getShort", "(Ljava/lang/Object;J)S", false, Native::Core::getI4);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "putShort", "(Ljava/lang/Object;JS)V", false, Native::Core::putI4);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "getChar", "(Ljava/lang/Object;J)C", false, Native::Core::getI4);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "putChar", "(Ljava/lang/Object;JC)V", false, Native::Core::putI4);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "getInt", "(Ljava/lang/Object;J)I", false, Native::Core::getI4);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "putInt", "(Ljava/lang/Object;JI)V", false, Native::Core::putI4);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "getLong", "(Ljava/lang/Object;J)J", false, Native::Core::getLong);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "putLong", "(Ljava/lang/Object;JJ)V", false, Native::Core::putLong);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "getFloat", "(Ljava/lang/Object;J)F", false, Native::Core::getFloat);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "putFloat", "(Ljava/lang/Object;JF)V", false, Native::Core::putFloat);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "getDouble", "(Ljava/lang/Object;J)D", false, Native::Core::getDouble);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "putDouble", "(Ljava/lang/Object;JD)V", false, Native::Core::putDouble);

        manager.regNativeMethod(UNSAFE_CLASS_NAME, "putOrderedObject", "(Ljava/lang/Object;JLjava/lang/Object;)V", false, Native::Core::putObject);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "putOrderedInt", "(Ljava/lang/Object;JI)V", false, Native::Core::putI4);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "putOrderedLong", "(Ljava/lang/Object;JJ)V", false, Native::Core::putLong);

        manager.regNativeMethod(UNSAFE_CLASS_NAME, "allocateMemory", "(J)J", false, Native::Core::cheapAllocateMemory);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "freeMemory", "(J)V", false, Native::Core::cheapFreeMemory);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "reallocateMemory", "(JJ)J", false, Native::Core::cheapReallocateMemory);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "setMemory", "(Ljava/lang/Object;JJB)V", false, Native::Core::cheapSetMemory);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "copyMemory", "(Ljava/lang/Object;JLjava/lang/Object;JJ)V", false, Native::Core::copyMemory);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "getByte", "(J)B", false, Native::Core::cheapGetByte);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "putByte", "(JB)V", false, Native::Core::cheapPutByte);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "getShort", "(J)S", false, Native::Core::cheapGetShort);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "putShort", "(JS)V", false, Native::Core::cheapPutShort);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "getChar", "(J)C", false, Native::Core::cheapGetChar16);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "putChar", "(JC)V", false, Native::Core::cheapPutChar16);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "getInt", "(J)I", false, Native::Core::cheapGetI4);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "putInt", "(JI)V", false, Native::Core::cheapPutI4);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "getLong", "(J)J", false, Native::Core::cheapGetI8);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "putLong", "(JJ)V", false, Native::Core::cheapPutI8);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "getFloat", "(J)F", false, Native::Core::cheapGetF4);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "putFloat", "(JF)V", false, Native::Core::cheapPutF4);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "getDouble", "(J)D", false, Native::Core::cheapGetF8);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "putDouble", "(JD)V", false, Native::Core::cheapPutF8);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "getAddress", "(J)J", false, Native::Core::cheapGetI8);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "putAddress", "(JJ)V", false, Native::Core::cheapPutI8);

        manager.regNativeMethod(UNSAFE_CLASS_NAME, "pageSize", "()I", false, Native::Core::pageSize);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "allocateInstance", "(Ljava/lang/Class;)Ljava/lang/Object;", false, Native::Core::allocateInstance);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "throwException", "(Ljava/lang/Throwable;)V", false, Native::Core::throwException);

        manager.regNativeMethod(UNSAFE_CLASS_NAME, "loadFence", "()V", false, Native::Core::loadFence);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "storeFence", "()V", false, Native::Core::storeFence);
        manager.regNativeMethod(UNSAFE_CLASS_NAME, "fullFence", "()V", false, Native::Core::fullFence);
    }

    void registerFloatCoreMethods(NativeManager &manager) {
        manager.regNativeMethod(JAVA_LANG_FLOAT_NAME, "floatToRawIntBits", "(F)I", false, Native::Core::floatToRawIntBits);
        manager.regNativeMethod(JAVA_LANG_FLOAT_NAME, "intBitsToFloat", "(I)F", false, Native::Core::intBitsToFloat);
        manager.regNativeMethod(JAVA_LANG_DOUBLE_NAME, "doubleToRawLongBits", "(D)J", false, Native::Core::doubleToRawLongBits);
        manager.regNativeMethod(JAVA_LANG_DOUBLE_NAME, "longBitsToDouble", "(J)D", false, Native::Core::longBitsToDouble);

        manager.regNativeMethod(JAVA_LANG_STRICT_MATH_NAME, "sin", "(D)D", false, Native::Core::_sin);
        manager.regNativeMethod(JAVA_LANG_STRICT_MATH_NAME, "cos", "(D)D", false, Native::Core::_cos);
        manager.regNativeMethod(JAVA_LANG_STRICT_MATH_NAME, "tan", "(D)D", false, Native::Core::_tan);
        manager.regNativeMethod(JAVA_LANG_STRICT_MATH_NAME, "asin", "(D)D", false, Native::Core::_asin);
        manager.regNativeMethod(JAVA_LANG_STRICT_MATH_NAME, "acos", "(D)D", false, Native::Core::_acos);
        manager.regNativeMethod(JAVA_LANG_STRICT_MATH_NAME, "atan", "(D)D", false, Native::Core::_atan);
        manager.regNativeMethod(JAVA_LANG_STRICT_MATH_NAME, "atan2", "(DD)D", false, Native::Core::_atan2);

        manager.regNativeMethod(JAVA_LANG_STRICT_MATH_NAME, "exp", "(D)D", false, Native::Core::_exp);
        manager.regNativeMethod(JAVA_LANG_STRICT_MATH_NAME, "log", "(D)D", false, Native::Core::_log);
        manager.regNativeMethod(JAVA_LANG_STRICT_MATH_NAME, "log10", "(D)D", false, Native::Core::_log10);
        manager.regNativeMethod(JAVA_LANG_STRICT_MATH_NAME, "sqrt", "(D)D", false, Native::Core::_sqrt);
        manager.regNativeMethod(JAVA_LANG_STRICT_MATH_NAME, "pow", "(DD)D", false, Native::Core::_pow);
        manager.regNativeMethod(JAVA_LANG_STRICT_MATH_NAME, "cbrt", "(D)D", false, Native::Core::_cbrt);
        manager.regNativeMethod(JAVA_LANG_STRICT_MATH_NAME, "IEEEremainder", "(DD)D", false, Native::Core::_ieeeremainder);

        manager.regNativeMethod(JAVA_LANG_STRICT_MATH_NAME, "sinh", "(D)D", false, Native::Core::_sinh);
        manager.regNativeMethod(JAVA_LANG_STRICT_MATH_NAME, "cosh", "(D)D", false, Native::Core::_cosh);
        manager.regNativeMethod(JAVA_LANG_STRICT_MATH_NAME, "tanh", "(D)D", false, Native::Core::_tanh);

        manager.regNativeMethod(JAVA_LANG_STRICT_MATH_NAME, "hypot", "(DD)D", false, Native::Core::_hypot);
        manager.regNativeMethod(JAVA_LANG_STRICT_MATH_NAME, "expm1", "(D)D", false, Native::Core::_expm1);
        manager.regNativeMethod(JAVA_LANG_STRICT_MATH_NAME, "log1p", "(D)D", false, Native::Core::_log1p);
    }

    void registerThrowableCoreMethods(NativeManager &manager) {
        manager.regNativeMethod(JAVA_LANG_THROWABLE_NAME, "fillInStackTrace", "(I)Ljava/lang/Throwable;", false, Native::Core::fillInStackTrace);
        manager.regNativeMethod(JAVA_LANG_THROWABLE_NAME, "getStackTraceDepth", "()I", false, Native::Core::getStackTraceDepth);
        manager.regNativeMethod(JAVA_LANG_THROWABLE_NAME, "getStackTraceElement", "(I)Ljava/lang/StackTraceElement;", false, Native::Core::getStackTraceElement);
    }

    void registerIOCoreMethods(NativeManager &manager) {
        manager.regNativeMethod(JAVA_IO_FILE_INPUT_STREAM_NAME, "open0", "(Ljava/lang/String;)V", false, Native::Core::readOpen);
        manager.regNativeMethod(JAVA_IO_FILE_INPUT_STREAM_NAME, "readBytes", "([BII)I", false, Native::Core::readBytes);
        manager.regNativeMethod(JAVA_IO_FILE_INPUT_STREAM_NAME, "close0", "()V", false, Native::Core::close0);
        manager.regNativeMethod(JAVA_IO_FILE_INPUT_STREAM_NAME, "read0", "()I", false, Native::Core::read0);
        manager.regNativeMethod(JAVA_IO_FILE_INPUT_STREAM_NAME, "skip0", "(J)J", false, Native::Core::skip0);
        manager.regNativeMethod(JAVA_IO_FILE_INPUT_STREAM_NAME, "available0", "()I", false, Native::Core::available0);

        manager.regNativeMethod(JAVA_IO_FILE_OUTPUT_STREAM_NAME, "open0", "(Ljava/lang/String;Z)V", false, Native::Core::writeOpen);
        manager.regNativeMethod(JAVA_IO_FILE_OUTPUT_STREAM_NAME, "writeBytes", "([BIIZ)V", false, Native::Core::writeBytes);
        manager.regNativeMethod(JAVA_IO_FILE_OUTPUT_STREAM_NAME, "write", "(IZ)V", false, Native::Core::write0);
        manager.regNativeMethod(JAVA_IO_FILE_OUTPUT_STREAM_NAME, "close0", "()V", false, Native::Core::close0);

        manager.regNativeMethod(JAVA_IO_UNIX_FILE_SYSTEM_NAME, "getBooleanAttributes0", "(Ljava/io/File;)I", false, Native::Core::getBooleanAttributes0);
        manager.regNativeMethod(JAVA_IO_UNIX_FILE_SYSTEM_NAME, "getLastModifiedTime", "(Ljava/io/File;)J", false, Native::Core::getLastModifiedTime);
        manager.regNativeMethod(JAVA_IO_UNIX_FILE_SYSTEM_NAME, "canonicalize0", "(Ljava/lang/String;)Ljava/lang/String;", false, Native::Core::canonicalize0);

        manager.regNativeMethod(JAVA_IO_OBJECT_STREAM_NAME, "initNative", "()V", false, Native::Core::initNative);
        manager.regNativeMethod(JAVA_IO_OBJECT_STREAM_NAME, "hasStaticInitializer", "(Ljava/lang/Class;)Z", false, Native::Core::hasStaticInitializer);
    }

    void registerAtomicCoreMethods(NativeManager &manager) {
        manager.regNativeMethod(JAVA_UTIL_CONCURRENT_ATOMIC_NAME, "VMSupportsCS8", "()Z", false, Native::Core::vmSupportsCS8);
    }

    void registerPackageCoreMethods(NativeManager &manager) {
        manager.regNativeMethod(JAVA_LANG_PACKAGE_NAME, "getSystemPackage0", "(Ljava/lang/String;)Ljava/lang/String;", false, Native::Core::getSystemPackage0);
    }

    void registerMiscCoreMethods(NativeManager &manager) {
        manager.regNativeMethod(JAVA_UTIL_TIME_ZONE_NAME, "getSystemTimeZoneID", "(Ljava/lang/String;)Ljava/lang/String;", false, Native::Core::getSystemTimeZoneID);
        manager.regNativeMethod("java/util/zip/CRC32", "updateBytes", "(I[BII)I", false, Native::Core::updateBytes);

        manager.regNativeMethod("java/lang/Shutdown", "beforeHalt", "()V", false, Native::Core::beforeHalt);
        manager.regNativeMethod("java/lang/Shutdown", "halt0", "(I)V", false, Native::Core::halt0);
    }

    void registerCoreMethods(NativeManager &manager) {
        registerObjectCoreMethods(manager);
        registerStringCoreMethods(manager);
        registerSystemCoreMethods(manager);
        registerThreadCoreMethods(manager);
        registerClassCoreMethods(manager);
        registerClassLoaderCoreMethods(manager);
        registerRuntimeCoreMethods(manager);
        registerInvokeCoreMethods(manager);
        registerReflectCoreMethods(manager);
        registerUnsafeCoreMethods(manager);
        registerFloatCoreMethods(manager);
        registerThrowableCoreMethods(manager);
        registerIOCoreMethods(manager);
        registerAtomicCoreMethods(manager);
        registerPackageCoreMethods(manager);
        registerMiscCoreMethods(manager);
    }

}