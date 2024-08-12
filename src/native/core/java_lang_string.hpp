#ifndef NATIVE_CORE_JAVA_LANG_STRING_HPP
#define NATIVE_CORE_JAVA_LANG_STRING_HPP
#include "../../config.hpp"
#include "../../vm.hpp"
#include "../../frame.hpp"
#include "../../string_pool.hpp"

namespace RexVM::Native::Core {

    void intern(Frame &frame) {
        const auto str = StringPool::getJavaString(frame.getThisInstance());
        const auto strRef = frame.mem.getInternString(str);
        frame.returnRef(strRef);
    }

}


#endif