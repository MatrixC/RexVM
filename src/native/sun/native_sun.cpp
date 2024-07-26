#include "../../config.hpp"
#include "../../basic_java_class.hpp"
#include "native_sun.hpp"

#include "sun_misc.hpp"

namespace RexVM::Native::Sun {

    void registerSunMethods(NativeManager &manager) {
        manager.regNativeMethod("sun/misc/URLClassPath", "getLookupCacheURLs", "(Ljava/lang/ClassLoader;)[Ljava/net/URL;", false, RexVM::Native::Sun::Msc::getLookupCacheURLs);
        manager.regNativeMethod("sun/misc/VM", "initialize", "()V", false, RexVM::Native::Sun::Msc::initialize);
        manager.regNativeMethod("sun/misc/Signal", "findSignal", "(Ljava/lang/String;)I", false, RexVM::Native::Sun::Msc::findSignal);
        manager.regNativeMethod("sun/misc/Signal", "handle0", "(IJ)J", false, RexVM::Native::Sun::Msc::handle0);
        manager.regNativeMethod("sun/reflect/NativeConstructorAccessorImpl", "newInstance0", "(Ljava/lang/reflect/Constructor;[Ljava/lang/Object;)Ljava/lang/Object;", false, RexVM::Native::Sun::Msc::newInstance0);
    }

}

