#include "string_utils.hpp"
#include "format.hpp"
#include <ranges>
#include <string>
#include "../exception.hpp"

namespace RexVM {
    std::vector<cview> splitString(cview str, const char delimiter) {

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

    cstring joinString(const std::vector<cstring> &str, const cstring &joiner) {
        if (str.empty()) {
            return {};
        }
        auto result = str[0];
        for (size_t i = 1; i < str.size(); ++i) {
            result.append(joiner);
            result.append(str[i]);
        }
        return result;
    }

    cstring joinString(const std::unordered_set<cstring> &str, const cstring &joiner) {
        if (str.empty()) {
            return {};
        }
        auto first = true;
        cstring result{};
        for (const auto &item : str) {
            result += first ? item : (joiner + item);
            first = false;
        }
        return result;
    }

    bool startWith(const cview str, const cview prefix) {
        if (prefix.size() > str.size()) {
            return false;
        }
        return str.substr(0, prefix.size()) == prefix;
    }

    bool endsWith(const cview str, const cview suffix) {
        if (suffix.size() > str.size()) {
            return false;
        }
        return str.substr(str.size() - suffix.size()) == suffix;
    }

    void ltrim(cstring &s) {
        s.erase(s.begin(), std::ranges::find_if(s, [](const unsigned char ch) {
            return !std::isspace(ch);
        }));
    }

    void rtrim(cstring &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), s.end());
    }

    cstring concatView(const cview str1, const cview str2) {
        cstring result(str1);
        result += str2;
        return result;
    }

    cstring concatView(const cview str1, const cview str2, const cview str3) {
        cstring result(str1);
        result += str2;
        result += str3;
        return result;
    }

    cstring replace(cstring src, const cstring& search, const cstring& replace) {
        size_t pos = 0;
        while ((pos = src.find(search, pos)) != std::string::npos) {
            src.replace(pos, search.length(), replace);
            pos += replace.length();
        }

        return src;
    }


    cstring utf16ToUtf8(const char16_t *utf16, size_t utf16Size) {
        cstring utf8;
        utf8.reserve(utf16Size * 2);

        for (size_t i = 0; i < utf16Size; ) {
            uint32_t codepoint;
            if (utf16[i] >= 0xD800 && utf16[i] <= 0xDBFF) {  // High surrogate
                if (i + 1 >= utf16Size) {
                    panic("Invalid UTF-16 sequence");
                }
                if (utf16[i + 1] < 0xDC00 || utf16[i + 1] > 0xDFFF) {
                    panic("Invalid UTF-16 sequence");
                }
                codepoint = ((utf16[i] - 0xD800) << 10) + (utf16[i + 1] - 0xDC00) + 0x10000;
                i += 2;
            } else {
                codepoint = utf16[i];
                ++i;
            }
            if (codepoint <= 0x7F) {
                utf8 += CAST_CHAR(codepoint);
            } else if (codepoint <= 0x7FF) {
                utf8 += CAST_CHAR((codepoint >> 6) | 0xC0);
                utf8 += CAST_CHAR((codepoint & 0x3F) | 0x80);
            } else if (codepoint <= 0xFFFF) {
                utf8 += CAST_CHAR((codepoint >> 12) | 0xE0);
                utf8 += CAST_CHAR(((codepoint >> 6) & 0x3F) | 0x80);
                utf8 += CAST_CHAR((codepoint & 0x3F) | 0x80);
            } else {
                utf8 += CAST_CHAR((codepoint >> 18) | 0xF0);
                utf8 += CAST_CHAR(((codepoint >> 12) & 0x3F) | 0x80);
                utf8 += CAST_CHAR(((codepoint >> 6) & 0x3F) | 0x80);
                utf8 += CAST_CHAR((codepoint & 0x3F) | 0x80);
            }
        }

        return utf8;
    }

    std::vector<char16_t> utf8ToUtf16Vec(const char *utf8, const size_t utf8Size) {
        std::vector<char16_t> utf16Vec;
        utf16Vec.reserve(utf8Size);
        for (size_t i = 0; i < utf8Size; ) {
            uint32_t codepoint;
            const size_t extraBytesToRead = (static_cast<uint8_t>(utf8[i]) >> 3 == 0b11110) ? 3 :
                                                (static_cast<uint8_t>(utf8[i]) >> 4 == 0b1110) ? 2 :
                                                    (static_cast<uint8_t>(utf8[i]) >> 5 == 0b110) ? 1 : 0;

            if (extraBytesToRead > 0) {
                codepoint = utf8[i] & (0xFF >> (extraBytesToRead + 2));
                for (size_t j = 1; j <= extraBytesToRead; ++j) {
                    if (i + j >= utf8Size) {
                        panic("Invalid UTF-16 sequence");
                    }
                    if ((utf8[i + j] & 0xC0) != 0x80) {
                        panic("Invalid UTF-16 sequence");
                    }
                    codepoint = (codepoint << 6) | (utf8[i + j] & 0x3F);
                }
                i += extraBytesToRead + 1;
            } else {
                codepoint = utf8[i];
                ++i;
            }

            if (codepoint <= 0xFFFF) {
                utf16Vec.emplace_back(codepoint);
            } else {
                codepoint -= 0x10000;
                utf16Vec.emplace_back(CAST_CHAR_16_T((codepoint >> 10) + 0xD800));
                utf16Vec.emplace_back(CAST_CHAR_16_T((codepoint & 0x3FF) + 0xDC00));
            }
        }

        return utf16Vec;
    }

    size_t utf8Length(const cview str) {
        size_t len = 0;
        for (const auto c : str) {
            if ((c & 0xC0) != 0x80) {
                ++len;
            }
        }
        return len;
    }
}