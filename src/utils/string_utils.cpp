#include "string_utils.hpp"
#include "format.hpp"
#include <ranges>
#include <string>

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

    cstring concat_view(cview str1, cview str2) {
        cstring result(str1);
        result += str2;
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
                if (i + 1 >= utf16Size) throw std::runtime_error("Invalid UTF-16 sequence");
                if (utf16[i + 1] < 0xDC00 || utf16[i + 1] > 0xDFFF) throw std::runtime_error("Invalid UTF-16 sequence");
                codepoint = ((utf16[i] - 0xD800) << 10) + (utf16[i + 1] - 0xDC00) + 0x10000;
                i += 2;
            } else {
                codepoint = utf16[i];
                ++i;
            }
            if (codepoint <= 0x7F) {
                utf8 += static_cast<char>(codepoint);
            } else if (codepoint <= 0x7FF) {
                utf8 += static_cast<char>((codepoint >> 6) | 0xC0);
                utf8 += static_cast<char>((codepoint & 0x3F) | 0x80);
            } else if (codepoint <= 0xFFFF) {
                utf8 += static_cast<char>((codepoint >> 12) | 0xE0);
                utf8 += static_cast<char>(((codepoint >> 6) & 0x3F) | 0x80);
                utf8 += static_cast<char>((codepoint & 0x3F) | 0x80);
            } else {
                utf8 += static_cast<char>((codepoint >> 18) | 0xF0);
                utf8 += static_cast<char>(((codepoint >> 12) & 0x3F) | 0x80);
                utf8 += static_cast<char>(((codepoint >> 6) & 0x3F) | 0x80);
                utf8 += static_cast<char>((codepoint & 0x3F) | 0x80);
            }
        }

        return utf8;
    }

    std::unique_ptr<char16_t[]> utf8ToUtf16(const char *utf8, size_t utf8Size) {
        const auto utf16Size = utf8Size;
        std::unique_ptr<char16_t[]> utf16Array = std::make_unique<char16_t[]>(utf16Size);
        size_t utf16Index = 0;

        for (size_t i = 0; i < utf8Size; ) {
            uint32_t codepoint;
            size_t extraBytesToRead = (uint8_t(utf8[i]) >> 3 == 0b11110) ? 3 :
                                      (uint8_t(utf8[i]) >> 4 == 0b1110) ? 2 :
                                      (uint8_t(utf8[i]) >> 5 == 0b110) ? 1 : 0;

            if (extraBytesToRead > 0) {
                codepoint = utf8[i] & (0xFF >> (extraBytesToRead + 2));
                for (size_t j = 1; j <= extraBytesToRead; ++j) {
                    if (i + j >= utf8Size) throw std::runtime_error("Invalid UTF-8 sequence");
                    if ((utf8[i + j] & 0xC0) != 0x80) throw std::runtime_error("Invalid UTF-8 sequence");
                    codepoint = (codepoint << 6) | (utf8[i + j] & 0x3F);
                }
                i += extraBytesToRead + 1;
            } else {
                codepoint = utf8[i];
                ++i;
            }

            if (codepoint <= 0xFFFF) {
                utf16Array[utf16Index++] = codepoint;
            } else {
                codepoint -= 0x10000;
                utf16Array[utf16Index++] = static_cast<char16_t>((codepoint >> 10) + 0xD800);
                utf16Array[utf16Index++] = static_cast<char16_t>((codepoint & 0x3FF) + 0xDC00);
            }
        }

        return utf16Array;
    }
}