#include "time.hpp"

namespace RexVM {

    i8 getCurrentTimeMillis() {
        const auto now = std::chrono::system_clock::now();
        const auto ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now).time_since_epoch().count();
        return CAST_I8(ms);
    }

    cstring millisecondsToReadableTime(const i8 timestamp) {
        const std::chrono::milliseconds ms(timestamp);
        const auto timePoint = std::chrono::time_point<std::chrono::system_clock>(ms);
        const std::time_t timeT = std::chrono::system_clock::to_time_t(timePoint);
        const auto tm = *std::localtime(&timeT);
        // return cformat("{:%Y-%m-%d %H:%M:%S}", tm);
        return cformat("{:%H:%M:%S}", tm);
    }
    
}