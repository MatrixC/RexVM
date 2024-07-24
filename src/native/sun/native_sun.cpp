#include "../../config.hpp"
#include "../../basic_java_class.hpp"
#include "native_sun.hpp"

#include "sun_misc.hpp"

namespace RexVM::Native::SUN {

    void registerSunMethods(NativeManager &manager) {
        manager.regNativeMethod("sun/misc/URLClassPath", "getLookupCacheURLs", "(Ljava/lang/ClassLoader;)[Ljava/net/URL;", false, Native::SUN::MISC::getLookupCacheURLs);
        manager.regNativeMethod("sun/misc/VM", "initialize", "()V", false, Native::SUN::MISC::initialize);
        manager.regNativeMethod("sun/misc/Signal", "findSignal", "(Ljava/lang/String;)I", false, Native::SUN::MISC::findSignal);
        manager.regNativeMethod("sun/misc/Signal", "handle0", "(IJ)J", false, Native::SUN::MISC::handle0);
        manager.regNativeMethod("sun/reflect/NativeConstructorAccessorImpl", "newInstance0", "(Ljava/lang/reflect/Constructor;[Ljava/lang/Object;)Ljava/lang/Object;", false, Native::SUN::MISC::newInstance0);
    }

}

