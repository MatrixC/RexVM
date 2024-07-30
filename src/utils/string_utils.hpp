#ifndef STRING_UTILS_HPP
#define STRING_UTILS_HPP

#include <vector>
#include <memory>
#include "../config.hpp"

namespace RexVM {

    std::vector<cview> splitString(cview str, char delimiter);
    cstring joinString(std::vector<cstring> strs, const cstring &joiner);

    bool startWith(cview str, cview prefix);
    bool endsWith(cview str, cview suffix);

    cstring concat_view(cview str1, cview str2);
    cstring concat_view(cview str1, cview str2, cview str3);

    cstring replace(cstring src, const cstring& search, const cstring& replace);

    cstring utf16ToUtf8(const char16_t *utf16, size_t utf16Size);
    std::vector<char16_t> utf8ToUtf16Vec(const char *utf8, size_t utf8Size);
    size_t utf8Length(const cstring &str);
}

#endif