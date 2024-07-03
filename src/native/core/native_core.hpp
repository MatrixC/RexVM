#ifndef NATIVE_CORE_HPP
#define NATIVE_CORE_HPP
#include "../native_manager.hpp"

namespace RexVM::Native::Core {

    void registerObjectCoreMethods(NativeManager &manager);
    void registerThreadCoreMethods(NativeManager &manager);
    void registerClassCoreMethods(NativeManager &manager);
    void registerRuntimeCoreMethods(NativeManager &manager);
    void registerInvokeCoreMethods(NativeManager &manager);
}

#endif
