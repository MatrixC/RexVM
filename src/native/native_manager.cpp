#include "native_manager.hpp"
#include "java_lang_object.hpp"
#include "java_lang_class.hpp"
#include "java_lang_system.hpp"
#include "java_lang_float.hpp"
#include "java_lang_string.hpp"
#include "java_lang_throwable.hpp"
#include "java_lang_thread.hpp"
#include "java_util_atomic.hpp"
#include "java_io_file_output_stream.hpp"
#include "java_security_accesscontroller.hpp"
#include "sun_misc_vm.hpp"
#include "self_native.hpp"


namespace RexVM {

    void nopMethod(Frame &frame) {
    }

    NativeManager::NativeManager() {
        regAllMethods();
    }

    NativeManager::~NativeManager() = default;

    NativeManager NativeManager::instance;

    void
    NativeManager::regNativeMethod(const cstring &className, const cstring &methodName, const cstring &descriptor,
                                   bool isStatic, NativeMethodHandler handler) {
        const auto key = className + ":" + methodName + ":" + descriptor;
        nativeMethods.emplace(key, handler);
    }

    NativeMethodHandler
    NativeManager::getNativeMethod(const cstring &className, const cstring &methodName,
                                   const cstring &descriptor, bool isStatic) {

        if (methodName == "registerNatives" || methodName == "initIDs") {
            return nopMethod;
        }

        const auto key = className + ":" + methodName + ":" + descriptor;
        if (const auto iter = nativeMethods.find(key); iter != nativeMethods.end()) {
            return iter->second;
        }
        return nullptr;
    }

    void NativeManager::regAllMethods() {
        regNativeMethod("Fto", "println", "(Ljava/lang/String;)V", true, Native::selfPrintln);
        regNativeMethod("Fto", "println", "(I)V", true, Native::selfPrintlnInt);

        regNativeMethod("java/lang/Object", "getClass", "()Ljava/lang/Class;", false, Native::getClass);
        regNativeMethod("java/lang/Object", "hashCode", "()I", false, Native::hashCode);
        regNativeMethod("java/lang/Object", "clone", "()Ljava/lang/Object;", false, Native::clone);
        regNativeMethod("java/lang/Object", "notifyAll", "()V", false, Native::notifyAll);
        regNativeMethod("java/lang/Object", "wait", "(J)V", false, nopMethod);

        regNativeMethod("java/lang/Class", "getPrimitiveClass", "(Ljava/lang/String;)Ljava/lang/Class;", true, Native::getPrimitiveClass);
        regNativeMethod("java/lang/Class", "desiredAssertionStatus0", "(Ljava/lang/Class;)Z", true, Native::desiredAssertionStatus0);
        regNativeMethod("java/lang/Class", "getName0", "()Ljava/lang/String;", true, Native::getName0);
        regNativeMethod("java/lang/Class", "getComponentType", "()Ljava/lang/Class;", true, Native::getComponentType);
        regNativeMethod("java/lang/Class", "forName0", "(Ljava/lang/String;ZLjava/lang/ClassLoader;Ljava/lang/Class;)Ljava/lang/Class;", true, Native::forName0);
        regNativeMethod("java/lang/Class", "getDeclaredFields0", "(Z)[Ljava/lang/reflect/Field;", true, Native::getDeclaredFields0);
        regNativeMethod("java/lang/Class", "getDeclaredConstructors0", "(Z)[Ljava/lang/reflect/Constructor;", true, Native::getDeclaredConstructors0);

        regNativeMethod("java/lang/Class", "isPrimitive", "()Z", true, Native::isPrimitive);
        regNativeMethod("java/lang/Class", "isInterface", "()Z", true, Native::isInterface);
        regNativeMethod("java/lang/Class", "isArray", "()Z", true, Native::isArray);
        regNativeMethod("java/lang/Class", "isAssignableFrom", "(Ljava/lang/Class;)Z", true, Native::isAssignableFrom);
        regNativeMethod("java/lang/Class", "getModifiers", "()I", true, Native::getModifiers);
        regNativeMethod("java/lang/Class", "getSuperclass", "()Ljava/lang/Class;", true, Native::getSuperclass);

        regNativeMethod("java/lang/System", "setIn0", "(Ljava/io/InputStream;)V", false, Native::setIn0);
        regNativeMethod("java/lang/System", "setOut0", "(Ljava/io/PrintStream;)V", false, Native::setOut0);
        regNativeMethod("java/lang/System", "setErr0", "(Ljava/io/PrintStream;)V", false, Native::setErr0);
        regNativeMethod("java/lang/System", "currentTimeMillis", "()J", false, Native::currentTimeMillis);
        regNativeMethod("java/lang/System", "initProperties", "(Ljava/util/Properties;)Ljava/util/Properties;", false, Native::initProperties);
        regNativeMethod("java/lang/System", "arraycopy", "(Ljava/lang/Object;ILjava/lang/Object;II)V", false, Native::arraycopy);
        regNativeMethod("java/lang/System", "mapLibraryName", "(Ljava/lang/String;)Ljava/lang/String;", false, Native::mapLibraryName);

        regNativeMethod("java/lang/Thread", "currentThread", "()Ljava/lang/Thread;", false, Native::currentThread);
        regNativeMethod("java/lang/Thread", "setPriority0", "(I)V", false, nopMethod);
        regNativeMethod("java/lang/Thread", "isAlive", "()Z", false, Native::isAlive);
        regNativeMethod("java/lang/Thread", "start0", "()V", false, Native::start0);

        regNativeMethod("java/lang/Throwable", "fillInStackTrace", "(I)Ljava/lang/Throwable;", false, Native::fillInStackTrace);

        regNativeMethod("java/lang/Float", "floatToRawIntBits", "(F)I", false, Native::floatToRawIntBits);
        regNativeMethod("java/lang/Float", "intBitsToFloat", "(I)F", false, Native::intBitsToFloat);
        regNativeMethod("java/lang/Double", "doubleToRawLongBits", "(D)J", false, Native::doubleToRawLongBits);
        regNativeMethod("java/lang/Double", "longBitsToDouble", "(J)D", false, Native::longBitsToDouble);
        regNativeMethod("java/lang/String", "intern", "()Ljava/lang/String;", false, Native::intern);

        regNativeMethod("java/io/FileOutputStream", "writeBytes", "([BIIZ)V", false, Native::writeBytes);

        regNativeMethod("sun/misc/VM", "initialize", "()V", false, Native::initialize);
        regNativeMethod("sun/misc/Unsafe", "arrayBaseOffset", "(Ljava/lang/Class;)I", false, Native::arrayBaseOffset);
        regNativeMethod("sun/misc/Unsafe", "arrayIndexScale", "(Ljava/lang/Class;)I", false, Native::arrayIndexScale);
        regNativeMethod("sun/misc/Unsafe", "addressSize", "()I", false, Native::addressSize);
        regNativeMethod("sun/misc/Unsafe", "objectFieldOffset", "(Ljava/lang/reflect/Field;)J", false, Native::objectFieldOffset);
        regNativeMethod("sun/reflect/Reflection", "getCallerClass", "()Ljava/lang/Class;", false, Native::getCallerClass);
        regNativeMethod("sun/reflect/Reflection", "getClassAccessFlags", "(Ljava/lang/Class;)I", false, Native::getClassAccessFlags);


        regNativeMethod("sun/misc/Unsafe", "compareAndSwapObject", "(Ljava/lang/Object;JLjava/lang/Object;Ljava/lang/Object;)Z", false, Native::compareAndSwapObject);
        regNativeMethod("sun/misc/Unsafe", "compareAndSwapInt", "(Ljava/lang/Object;JII)Z", false, Native::compareAndSwapInt);
        regNativeMethod("sun/misc/Unsafe", "compareAndSwapLong", "(Ljava/lang/Object;JJJ)Z", false, Native::compareAndSwapLong);
        regNativeMethod("sun/misc/Unsafe", "getIntVolatile", "(Ljava/lang/Object;J)I", false, Native::getIntVolatile);
        regNativeMethod("sun/misc/Unsafe", "getObjectVolatile", "(Ljava/lang/Object;J)Ljava/lang/Object;", false, Native::getObjectVolatile);
        regNativeMethod("sun/misc/Unsafe", "allocateMemory", "(J)J", false, Native::allocateMemory);
        regNativeMethod("sun/misc/Unsafe", "putLong", "(JJ)V", false, Native::putLong);
        regNativeMethod("sun/misc/Unsafe", "getByte", "(J)B", false, Native::getByte);
        regNativeMethod("sun/misc/Unsafe", "freeMemory", "(J)V", false, Native::freeMemory);

        regNativeMethod("sun/misc/Signal", "findSignal", "(Ljava/lang/String;)I", false, Native::findSignal);
        regNativeMethod("sun/misc/Signal", "handle0", "(IJ)J", false, Native::handle0);

        regNativeMethod("java/security/AccessController", "doPrivileged", "(Ljava/security/PrivilegedAction;)Ljava/lang/Object;", false, Native::doPrivileged);
        regNativeMethod("java/security/AccessController", "doPrivileged", "(Ljava/security/PrivilegedAction;Ljava/security/AccessControlContext;)Ljava/lang/Object;", false, Native::doPrivileged);
        regNativeMethod("java/security/AccessController", "doPrivileged", "(Ljava/security/PrivilegedExceptionAction;)Ljava/lang/Object;", false, Native::doPrivileged);
        regNativeMethod("java/security/AccessController", "doPrivileged", "(Ljava/security/PrivilegedExceptionAction;Ljava/security/AccessControlContext;)Ljava/lang/Object;", false, Native::doPrivileged);
        regNativeMethod("java/security/AccessController", "doPrivileged", "(Ljava/security/PrivilegedAction;)Ljava/lang/Object;", false, Native::doPrivileged);
        regNativeMethod("java/security/AccessController", "getStackAccessControlContext", "()Ljava/security/AccessControlContext;", false, Native::getStackAccessControlContext);

        regNativeMethod("sun/reflect/NativeConstructorAccessorImpl", "newInstance0", "(Ljava/lang/reflect/Constructor;[Ljava/lang/Object;)Ljava/lang/Object;", false, Native::newInstance0);

        regNativeMethod("java/util/concurrent/atomic/AtomicLong", "VMSupportsCS8", "()Z", false, Native::vmSupportsCS8);
    }


}