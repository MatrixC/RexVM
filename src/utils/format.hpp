#ifndef FORMAT_HPP
#define FORMAT_HPP

#define FMT_HEADER_ONLY
#include <fmt/core.h>
#include <fmt/chrono.h>

namespace RexVM {

    template<typename... T>
    FMT_NODISCARD FMT_INLINE auto cformat(fmt::format_string<T...> fmt, T &&... args)
    -> std::string {
        return fmt::format(fmt, std::forward<T>(args)...);
    }

    template<typename... T>
    FMT_INLINE void cprint(fmt::format_string<T...> fmt, T &&... args) {
        return fmt::print(fmt, std::forward<T>(args)...);
    }

    template<typename... T>
    FMT_INLINE void cprintln(fmt::format_string<T...> fmt, T &&... args) {
        return fmt::println(fmt, std::forward<T>(args)...);
    }

    template<typename... T>
    FMT_INLINE void cprintlnErr(fmt::format_string<T...> fmt, T &&... args) {
        return fmt::println(stderr, fmt, std::forward<T>(args)...);
    }
}

#endif
