#ifndef STRING_UTILS_HPP
#define STRING_UTILS_HPP

#include <vector>
#include "../config.hpp"

namespace RexVM {

    std::vector<cview> split_string(cview str, char delimiter);

    bool endsWith(cview str, cview suffix);

    cstring concat_view(cview str1, cview str2);

    cstring replace(cstring src, const cstring& search, const cstring& replace);

    cstring utf16ToUtf8(const char16_t *utf16, size_t utf16Size);
    std::unique_ptr<char16_t[]> utf8ToUtf16(const char *utf8, size_t utf8Size);
}

#endif