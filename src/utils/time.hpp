#ifndef TIME_HPP
#define TIME_HPP
#include "../config.hpp"

namespace RexVM {

    i8 getCurrentTimeMillis();

    cstring millisecondsToReadableTime(i8 timestamp);

}

#endif