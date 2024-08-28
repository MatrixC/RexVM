#ifndef NATIVE_MANAGER_HPP
#define NATIVE_MANAGER_HPP

#include <unordered_map>
#include "../config.hpp"

namespace RexVM {

    struct NativeManager {

        std::unordered_map<cstring, NativeMethodHandler> nativeMethods;

        void regNativeMethod(
                const cstring &className, const cstring &methodName, const cstring &descriptor,
                bool isStatic, NativeMethodHandler handler
        );

        NativeMethodHandler getNativeMethod(
                const cstring &className, const cstring &methodName, const cstring &descriptor, bool isStatic
        );

        //static safe
        static NativeManager instance;

        explicit NativeManager();
        ~NativeManager();

        void regAllMethods();

    };
}

#endif
