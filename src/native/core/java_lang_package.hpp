#ifndef NATIVE_CORE_JAVA_LANG_PACKAGE_HPP
#define NATIVE_CORE_JAVA_LANG_PACKAGE_HPP
#include "../../config.hpp"
#include "../../vm.hpp"
#include "../../frame.hpp"
#include "../../string_pool.hpp"

namespace RexVM::Native::Core {

    void getSystemPackage0(Frame &frame) {
        frame.returnRef(nullptr);
    }

}


#endif