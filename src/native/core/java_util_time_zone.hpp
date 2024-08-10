#ifndef NATIVE_CORE_JAVA_UTIL_TIME_ZONE_HPP
#define NATIVE_CORE_JAVA_UTIL_TIME_ZONE_HPP
#include "../../config.hpp"
#include "../../vm.hpp"
#include "../../frame.hpp"
#include "../../thread.hpp"
#include "../../string_pool.hpp"

namespace RexVM::Native::Core {

    void getSystemTimeZoneID(Frame &frame) {
        frame.returnRef(frame.vm.stringPool->getInternString(getSystemTimeZoneId()));
    }


}

#endif