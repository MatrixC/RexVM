#ifndef OS_PLATFORM_HPP
#define OS_PLATFORM_HPP

#if defined(__clang__) || defined(__GNUC__)
#include "platform/compiler_clang.hpp"
#elif defined(_MSC_VER)
#include "platform/compiler_msvc.hpp"
#else
#error not support compiler
#endif

#if defined(_WIN32)
#include "platform/os_windows.hpp"
#elif defined(__APPLE__) || defined(__linux__)
#include "platform/os_unix_like.hpp"
#else
#error not support system
#endif

#if defined(__x86_64__) || defined(_M_X64)
#include "platform/arch_amd64.hpp"
#elif defined(__aarch64__) || defined(_M_ARM64)
#include "platform/arch_arm64.hpp"
#else
#error not support arch
#endif

#include <string>

namespace RexVM {
    std::size_t getSystemPageSize();
    std::string getSystemTimeZoneId();
    void setThreadName(const char *name);
}

#endif