#ifndef STRING_UTILS_HPP
#define STRING_UTILS_HPP

#include <vector>
#include "../config.hpp"

namespace RexVM {

    std::vector<cview> split_string(cview str, char delimiter);

    bool endsWith(cview str, cview suffix);

    cstring concat_view(cview str1, cview str2);

    cstring u16charsToString(const cchar_16 *str, size_t length);

    ustring stringToUString(const cstring &str);

    cstring replace(cstring src, const cstring& search, const cstring& replace);
}

#endif