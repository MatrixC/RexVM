#ifndef NATIVE_MANAGER_HPP
#define NATIVE_MANAGER_HPP

#include <hash_table8.hpp>
#include "../config.hpp"

namespace RexVM {

    struct NativeManager {

        emhash8::HashMap<cstring, NativeMethodHandler> nativeMethods;

        void regNativeMethod(
                cview className, cview methodName, cview descriptor,
                bool isStatic, NativeMethodHandler handler
        );

        NativeMethodHandler getNativeMethod(
                cview className, cview methodName, cview descriptor, bool isStatic
        );

        //static safe
        static NativeManager instance;

        explicit NativeManager();
        ~NativeManager();

        void regAllMethods();

    };
}

#endif
