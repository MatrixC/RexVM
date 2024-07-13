#include "native_manager.hpp"
#include "../basic_java_class.hpp"
#include "java_lang_system.hpp"
#include "java_lang_float.hpp"
#include "java_lang_throwable.hpp"
#include "java_util_atomic.hpp"
#include "java_io_file_output_stream.hpp"
#include "java_security_accesscontroller.hpp"
#include "sun_misc_vm.hpp"
#include "self_native.hpp"
#include "core/native_core.hpp"
#include "sun/native_sun.hpp"


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
       
        


        regNativeMethod(JAVA_LANG_SYSTEM_NAME, "setIn0", "(Ljava/io/InputStream;)V", false, Native::setIn0);
        regNativeMethod(JAVA_LANG_SYSTEM_NAME, "setOut0", "(Ljava/io/PrintStream;)V", false, Native::setOut0);
        regNativeMethod(JAVA_LANG_SYSTEM_NAME, "setErr0", "(Ljava/io/PrintStream;)V", false, Native::setErr0);
        regNativeMethod(JAVA_LANG_SYSTEM_NAME, "currentTimeMillis", "()J", false, Native::currentTimeMillis);
        regNativeMethod(JAVA_LANG_SYSTEM_NAME, "initProperties", "(Ljava/util/Properties;)Ljava/util/Properties;", false, Native::initProperties);
        regNativeMethod(JAVA_LANG_SYSTEM_NAME, "arraycopy", "(Ljava/lang/Object;ILjava/lang/Object;II)V", false, Native::arraycopy);
        regNativeMethod(JAVA_LANG_SYSTEM_NAME, "mapLibraryName", "(Ljava/lang/String;)Ljava/lang/String;", false, Native::mapLibraryName);

        regNativeMethod(JAVA_LANG_THROWABLE_NAME, "fillInStackTrace", "(I)Ljava/lang/Throwable;", false, Native::fillInStackTrace);

        regNativeMethod(JAVA_LANG_FLOAT_NAME, "floatToRawIntBits", "(F)I", false, Native::floatToRawIntBits);
        regNativeMethod(JAVA_LANG_FLOAT_NAME, "intBitsToFloat", "(I)F", false, Native::intBitsToFloat);
        regNativeMethod(JAVA_LANG_DOUBLE_NAME, "doubleToRawLongBits", "(D)J", false, Native::doubleToRawLongBits);
        regNativeMethod(JAVA_LANG_DOUBLE_NAME, "longBitsToDouble", "(J)D", false, Native::longBitsToDouble);

        regNativeMethod("java/io/FileOutputStream", "writeBytes", "([BIIZ)V", false, Native::writeBytes);

        regNativeMethod("sun/misc/VM", "initialize", "()V", false, Native::initialize);

        
        regNativeMethod("sun/misc/Unsafe", "putLong", "(JJ)V", false, Native::putLong);
        regNativeMethod("sun/misc/Unsafe", "getByte", "(J)B", false, Native::getByte);

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

        Native::Core::registerCoreMethods(*this);
        Native::SUN::registerSunMethods(*this);
    }


}