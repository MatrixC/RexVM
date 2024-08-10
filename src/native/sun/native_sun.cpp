#include "../../config.hpp"
#include "../../basic_java_class.hpp"
#include "native_sun.hpp"

#include "sun_misc.hpp"

namespace RexVM::Native::Sun {

    void registerSunMethods(NativeManager &manager) {
        manager.regNativeMethod("sun/misc/URLClassPath", "getLookupCacheURLs", "(Ljava/lang/ClassLoader;)[Ljava/net/URL;", false, RexVM::Native::Sun::Misc::getLookupCacheURLs);
        manager.regNativeMethod("sun/misc/VM", "initialize", "()V", false, RexVM::Native::Sun::Misc::initialize);
        manager.regNativeMethod("sun/misc/Signal", "findSignal", "(Ljava/lang/String;)I", false, RexVM::Native::Sun::Misc::findSignal);
        manager.regNativeMethod("sun/misc/Signal", "handle0", "(IJ)J", false, RexVM::Native::Sun::Misc::handle0);
        manager.regNativeMethod("sun/reflect/NativeConstructorAccessorImpl", "newInstance0", "(Ljava/lang/reflect/Constructor;[Ljava/lang/Object;)Ljava/lang/Object;", false, RexVM::Native::Sun::Misc::newInstance0);
        manager.regNativeMethod("sun/misc/Perf", "createLong", "(Ljava/lang/String;IIJ)Ljava/nio/ByteBuffer;", false, RexVM::Native::Sun::Misc::createLong);

        manager.regNativeMethod("sun/nio/fs/UnixNativeDispatcher", "init", "()I", false, RexVM::Native::Sun::Misc::undInit);
        manager.regNativeMethod("sun/nio/fs/UnixNativeDispatcher", "getcwd", "()[B", false, RexVM::Native::Sun::Misc::undGetCwd);
    }

}

