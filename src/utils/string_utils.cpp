#include "string_utils.hpp"
#include "format.hpp"
#include <ranges>
#include <string>
#include <codecvt>
#include <locale>

namespace RexVM {
    std::vector<cview> split_string(cview str, char delimiter) {

        std::vector<cview> result;
        size_t start = 0;
        size_t end = str.find(delimiter);

        while (end != cview::npos) {
            result.push_back(str.substr(start, end - start));
            start = end + 1;
            end = str.find(delimiter, start);
        }

        result.push_back(str.substr(start, end));
        return result;
    }

    bool endsWith(cview str, cview suffix) {
        if (suffix.size() > str.size()) {
            return false;
        }
        return str.substr(str.size() - suffix.size()) == suffix;
    }

    bool endsWith(cview str, char suffix) {
        return str[str.length() - 1] == suffix;
    }

    cstring concat_view(cview str1, cview str2) {
        cstring result(str1);
        result += str2;
        return result;
    }

    cstring u16charsToString(const cchar_16 *str, size_t length) {
        std::u16string u16String(str, length);
        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
        return convert.to_bytes(u16String);
    }

    ustring stringToUString(const cstring &str) {
        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
        return convert.from_bytes(str);
    }

    cstring replace(cstring src, cstring search, cstring replace) {
        size_t pos = 0;
        while ((pos = src.find(search, pos)) != std::string::npos) {
            src.replace(pos, search.length(), replace);
            pos += replace.length();
        }

        return src;
    }

}