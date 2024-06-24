//
// Created by ch on 12/20/23.
//

#ifndef FORMAT_HPP
#define FORMAT_HPP

#define FMT_HEADER_ONLY
#include <fmt/core.h>

namespace RexVM {

    template<typename... T>
    FMT_NODISCARD FMT_INLINE auto format(fmt::format_string<T...> fmt, T &&... args)
    -> std::string {
        return fmt::format(fmt, std::forward<T>(args)...);
    }

    template<typename... T>
    FMT_INLINE void print(fmt::format_string<T...> fmt, T &&... args) {
        return fmt::print(fmt, std::forward<T>(args)...);
    }

    template<typename... T>
    FMT_INLINE void println(fmt::format_string<T...> fmt, T &&... args) {
        return fmt::println(fmt, std::forward<T>(args)...);
    }
}

#endif
