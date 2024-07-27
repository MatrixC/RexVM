#include "native_manager.hpp"
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
        Native::Core::registerCoreMethods(*this);
        Native::Sun::registerSunMethods(*this);
    }


}