#include "composite_string.hpp"
#include "exception.hpp"
#include <cstring>

namespace RexVM {

    CompositeString CompositeString::EMPTY{nullptr};

    CompositeString::CompositeString() = default;

    CompositeString::CompositeString(const Char *str, const CommonSize size) {
        if (str == nullptr || size == 0) {
            return;
        }
        if (size > COM_DATA_SHIFT_MASK) {
            panic("not support length");
        }
        copy(str, size);
    }

    CompositeString::CompositeString(const Char16 *str, const CommonSize size) {
        const auto newSize = size * 2;
        if (newSize > COM_DATA_SHIFT_MASK) {
            panic("not support length");
        }
        const auto newStr = new Char[newSize];
        
        CommonSize newStrIdx = 0;
        for (CommonSize i = 0; i < size; ) {
            std::uint32_t codepoint;
            if (str[i] >= 0xD800 && str[i] <= 0xDBFF) {  // High surrogate
                if (i + 1 >= size) {
                    panic("Invalid UTF-16 sequence");
                }
                if (str[i + 1] < 0xDC00 || str[i + 1] > 0xDFFF) {
                    panic("Invalid UTF-16 sequence");
                }
                codepoint = ((str[i] - 0xD800) << 10) + (str[i + 1] - 0xDC00) + 0x10000;
                i += 2;
            } else {
                codepoint = str[i];
                ++i;
            }
            if (codepoint <= 0x7F) {
                newStr[newStrIdx++] += static_cast<Char>(codepoint);
            } else if (codepoint <= 0x7FF) {
                newStr[newStrIdx++] += static_cast<Char>((codepoint >> 6) | 0xC0);
                newStr[newStrIdx++] += static_cast<Char>((codepoint & 0x3F) | 0x80);
            } else if (codepoint <= 0xFFFF) {
                newStr[newStrIdx++] += static_cast<Char>((codepoint >> 12) | 0xE0);
                newStr[newStrIdx++] += static_cast<Char>(((codepoint >> 6) & 0x3F) | 0x80);
                newStr[newStrIdx++] += static_cast<Char>((codepoint & 0x3F) | 0x80);
            } else {
                newStr[newStrIdx++] += static_cast<Char>((codepoint >> 18) | 0xF0);
                newStr[newStrIdx++] += static_cast<Char>(((codepoint >> 12) & 0x3F) | 0x80);
                newStr[newStrIdx++] += static_cast<Char>(((codepoint >> 6) & 0x3F) | 0x80);
                newStr[newStrIdx++] += static_cast<Char>((codepoint & 0x3F) | 0x80);
            }
        }

        _data.reset(newStr, static_cast<CompositeStringSize>(newSize));
    }

    CompositeString::CompositeString(const Char *str) : CompositeString(str, str == nullptr ? 0 : std::strlen(str)) {
    }

    CompositeString::CompositeString(const std::string &str) : CompositeString(str.c_str(), str.size()) {
    }

    CompositeString::CompositeString(const CompositeString &other) {
        if (other.size() == 0){ 
            return;
        }
        copy(other);
    }

    CompositeString::CompositeString(const Char *str1, CommonSize size1, const Char *str2, CommonSize size2) {
        size1 = str1 == nullptr ? 0 : size1;
        size2 = str2 == nullptr ? 0 : size2;
        const auto newSize = size1 + size2;
        if (newSize == 0) {
            return;
        }
        if (newSize > COM_DATA_SHIFT_MASK) {
            panic("not support length");
        }
        const auto newStr = new Char[newSize];
        auto otherStrPtr = newStr;
        if (size1 != 0) {
            std::memcpy(newStr, str1, CharSize * size1);
            otherStrPtr += size1;
        }
        if (size2 != 0) {
            std::memcpy(otherStrPtr, str2, CharSize * size2);
        }
        _data.reset(newStr, static_cast<CompositeStringSize>(newSize));
    }


    CompositeString::CompositeString(const std::string_view &str1, const std::string_view &str2) :
        CompositeString(str1.data(), str1.size(), str2.data(), str2.size()) {
    }

    CompositeString::CompositeString(CompositeString &&other) noexcept {
        _data.composite = other._data.composite;
        other._data.reset();
    }

    CompositeString::~CompositeString() {
        release();
    }

    void CompositeString::copy(const Char * str, CommonSize size) {
        const auto copy = new Char[size];
        std::memcpy(copy, str, CharSize * size);
        _data.reset(copy, static_cast<CompositeStringSize>(size));
    }

    void CompositeString::copy(const CompositeString &other) {
        copy(other.c_str(), other.size());
    }

    void CompositeString::release() {
        delete[] _data.getPtr();
        _data.reset();
    }

    void CompositeString::reserve(CommonSize size) {
        if (c_str() != nullptr) {
            panic("already allocate memory");
        }
        if (size > COM_DATA_SHIFT_MASK) {
            panic("not support length");
        }

        const auto newStr = new Char[size];
        _data.reset(newStr, static_cast<CompositeStringSize>(size));
    }

    bool CompositeString::eq(const Char *other, CommonSize otherSize) const {
        const auto selfSize = size();
        if (selfSize != otherSize) {
            return false;
        }

        const auto selfPtr = c_str();
        if (selfPtr == other) {
            return true;
        } else if (selfPtr == nullptr || other == nullptr) {
            return false;
        } else {
            return std::memcmp(selfPtr, other, selfSize) == 0;
        }
    }

    bool CompositeString::lt(const Char *other, CommonSize otherSize) const {
        //"" < ""     false
        //"" < "any"  true
        const auto selfSize = size();
        if (selfSize == 0) {
            return otherSize != 0;
        }
        if (otherSize == 0) {
            return false;
        }
        const auto minSize = std::min(selfSize, otherSize);
        return strncmp(c_str(), other, minSize) < 0;
    }

    const CompositeString::Char &CompositeString::at(CommonSize pos) const {
        if (pos >= size()) {
            panic("error index");
        }
        return *(c_str() + pos);
    }

    const CompositeString::Char &CompositeString::front() const {
        return at(0);
    }

    const CompositeString::Char &CompositeString::back() const {
        return at(size() - 1);
    }

    const CompositeString::Char &CompositeString::operator[](CommonSize pos) const {
        return at(pos);
    }

    const CompositeString::Char *CompositeString::c_str() const {
        return _data.getPtr();
    }

    CompositeString::Char *CompositeString::data() const {
        return const_cast<CompositeString::Char *>(_data.getPtr());
    }

    bool CompositeString::empty() const {
        return size() == 0;
    }

    CompositeString::CommonSize CompositeString::size() const {
        return static_cast<CommonSize>(_data.getData());
    }

    CompositeString::CommonSize CompositeString::unicodeSize() const {
        CommonSize usize = 0;
        const auto cstr = c_str();
        for (CommonSize i = 0; i < size(); ++i) {
            if ((cstr[i] & 0xC0) != 0x80) {
                ++usize;
            }
        }
        return usize;
    }

    CompositeString &CompositeString::operator=(const CompositeString &other) {
        if (this != &other) {
            release();
            copy(other);
        }
        return *this;
    }

    CompositeString CompositeString::operator+(const CompositeString &other) const {
        const auto selfSize = size();
        const auto otherSize = other.size();
        const auto newSize = selfSize + otherSize;
        if (newSize == 0) {
            return EMPTY;
        }

        return {c_str(), selfSize, other.c_str(), otherSize};
    }

    bool CompositeString::operator==(const Char *other) const {
        const auto otherSize = other == nullptr ? 0 : strlen(other);
        return eq(other, otherSize);
    }

    bool CompositeString::operator<(const Char *other) const {
        const auto otherSize = other == nullptr ? 0 : strlen(other);
        return lt(other, otherSize);
    }

    bool CompositeString::operator==(const CompositeString &other) const {
        return eq(other.c_str(), other.size());
    }

    bool CompositeString::operator<(const CompositeString &other) const {
        return lt(other.c_str(), other.size());
    }

    bool CompositeString::operator==(const std::string_view &view) const {
        return eq(view.data(), view.size());
    }

    bool CompositeString::operator<(const std::string_view &view) const {
        return lt(view.data(), view.size());
    }

    CompositeString::HashType CompositeString::hashCode() const {
        // constexpr HashType seed = 131;
        // HashType hash = 0;
        // auto ptr = c_str();

        // for (std::size_t i = 0; i < size(); ++ptr, ++i) {
        //     hash = (hash * seed) + (*ptr);
        // }

        // return hash;
        //和std::string 保持一致 方便在string_pool中做管理
        return HashViewFunction{}(toStringView());
    }

    std::string CompositeString::toString() const {
        return {c_str(), size()};
    }

    std::string_view CompositeString::toStringView() const {
        return {c_str(), size()};
    }

    std::ostream& operator<<(std::ostream& out, const CompositeString &str) {
	    out << str.toStringView();
	    return out;
    }

}