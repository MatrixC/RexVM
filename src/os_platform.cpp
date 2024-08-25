#include "os_platform.hpp"
#include <array>
#include <cstring>
#if defined(_MSC_VER)
#include <Windows.h>
#else
#include <unistd.h>
#include <pthread.h>
#endif


namespace RexVM {

    std::size_t getSystemPageSize() {
        std::size_t pageSize{0};

#if defined(_MSC_VER)
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        pageSize = static_cast<std::size_t>(si.dwPageSize);
#else
        pageSize = static_cast<std::size_t>(getpagesize());
#endif

        return pageSize;
    }

    std::string getSystemTimeZoneId() {
        std::string timeZoneId;
    
#if defined(_MSC_VER)
        TIME_ZONE_INFORMATION tzInfo;
        if (GetTimeZoneInformation(&tzInfo) != TIME_ZONE_ID_INVALID) {
            wchar_t *wTimeZoneName = tzInfo.StandardName;
            char timeZoneName[32];
            wcstombs(timeZoneName, wTimeZoneName, sizeof(timeZoneName));
            timeZoneId = timeZoneName;
        }
#else
        const auto pipe = popen("readlink /etc/localtime", "r");
        if (!pipe) {
            return "Unknown";
        }
        std::array<char, 128> buffer{};
        std::string result{};
        while (fgets(buffer.data(), sizeof(buffer), pipe) != nullptr) {
            result += buffer.data();
        }
        pclose(pipe);
        size_t pos = result.find("/zoneinfo/");
        if (pos != std::string::npos) {
            timeZoneId = result.substr(pos + strlen("/zoneinfo/"));
            timeZoneId.erase(timeZoneId.find_last_not_of(" \n\r\t") + 1);
        }
#endif
        return timeZoneId;
    }

    void setThreadName(const char *name) {
#if defined(_MSC_VER)
#elif defined(__APPLE__)
        pthread_setname_np(name);
#else
        pthread_setname_np(pthread_self(), name);
#endif
    }

}