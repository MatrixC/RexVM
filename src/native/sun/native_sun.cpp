#include "../../config.hpp"
#include "../../basic_java_class.hpp"
#include "native_sun.hpp"

#include "sun_misc.hpp"

namespace RexVM::Native::SUN {

    void registerSunMethods(NativeManager &manager) {
        manager.regNativeMethod("sun/misc/URLClassPath", "getLookupCacheURLs", "(Ljava/lang/ClassLoader;)[Ljava/net/URL;", false, Native::SUN::MISC::getLookupCacheURLs);
    }

}

