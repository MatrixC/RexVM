#include "native_manager.hpp"
#include "core/native_core.hpp"
#include "sun/native_sun.hpp"
#include "misc/native_misc.hpp"
#include "rex/native_rex.hpp"


namespace RexVM {

    void nopMethod(Frame &frame) {
    }

    NativeManager::NativeManager() {
        regAllMethods();
    }

    NativeManager::~NativeManager() = default;

    NativeManager NativeManager::instance;

    void
    NativeManager::regNativeMethod(cview className, cview methodName, cview descriptor,
                                   bool isStatic, NativeMethodHandler handler) {
        const auto key = cformat("{}:{}:{}", className, methodName, descriptor);
        nativeMethods.emplace(key, handler);
    }

    NativeMethodHandler
    NativeManager::getNativeMethod(cview className, cview methodName,
                                   cview descriptor, bool isStatic) {

        if (methodName == "registerNatives" || methodName == "initIDs") {
            return nopMethod;
        }

        const auto key = cformat("{}:{}:{}", className, methodName, descriptor);
        if (const auto iter = nativeMethods.find(key); iter != nativeMethods.end()) {
            return iter->second;
        }
        return nullptr;
    }

    void NativeManager::regAllMethods() {
        Native::Core::registerCoreMethods(*this);
        Native::Misc::registerMiscMethods(*this);
        Native::Sun::registerSunMethods(*this);
        Native::Rex::registerRexMethods(*this);
    }


}