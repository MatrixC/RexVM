#ifndef NATIVE_CORE_JAVA_UTIL_TIME_ZONE_HPP
#define NATIVE_CORE_JAVA_UTIL_TIME_ZONE_HPP
#include <iostream>
#include <chrono>
#include <ctime>
#include <string>
#if defined(_WIN32)
#include <windows.h>
#endif
#include "../../config.hpp"
#include "../../vm.hpp"
#include "../../frame.hpp"
#include "../../thread.hpp"
#include "../../string_pool.hpp"

namespace RexVM::Native::Core {

    void getSystemTimeZoneID(Frame &frame) {
        cstring timeZoneID;
    
#if defined(_WIN32)
        TIME_ZONE_INFORMATION tzInfo;
        if (GetTimeZoneInformation(&tzInfo) != TIME_ZONE_ID_INVALID) {
            wchar_t *wTimeZoneName = tzInfo.StandardName;
            char timeZoneName[32];
            wcstombs(timeZoneName, wTimeZoneName, sizeof(timeZoneName));
            timeZoneID = timeZoneName;
        }
#else
        const auto pipe = popen("readlink /etc/localtime", "r");
        if (!pipe) {
            frame.returnRef(frame.vm.stringPool->getInternString("Unknown"));
            return;
        }
        char buffer[128];
        cstring result{};
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            result += buffer;
        }
        pclose(pipe);
        size_t pos = result.find("/zoneinfo/");
        if (pos != cstring::npos) {
            timeZoneID = result.substr(pos + strlen("/zoneinfo/"));
            timeZoneID.erase(timeZoneID.find_last_not_of(" \n\r\t") + 1);
        }
#endif
        frame.returnRef(frame.vm.stringPool->getInternString(timeZoneID));
    }


}

#endif