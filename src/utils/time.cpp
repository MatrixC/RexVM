#include "time.hpp"

namespace RexVM {

    i8 getCurrentTimeMillis() {
        const auto now = std::chrono::system_clock::now();
        const auto ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now).time_since_epoch().count();
        return CAST_I8(ms);
    }


}