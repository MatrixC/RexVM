#ifndef COMPOSITE_STRING_HPP
#define COMPOSITE_STRING_HPP
#include "composite_ptr.hpp"
#include <string>
#include <ostream>
#include <fmt/core.h>

namespace RexVM {

    struct CompositeString {
        using Char = char;
        using Char16 = char16_t;
        using HashType = std::size_t;
        using CompositeStringSize = std::uint16_t;
        using CommonSize = std::size_t;
        using HashViewFunction = std::hash<std::string_view>;

        static constexpr std::size_t CharSize = sizeof(Char);
        static CompositeString EMPTY;

        Composite<const Char *, CompositeStringSize> _data;

        CompositeString();
        CompositeString(const Char *str, CommonSize size);
        CompositeString(const Char16 *str, CommonSize size);
        explicit CompositeString(const Char *str);
        explicit CompositeString(const std::string &str);
        CompositeString(const CompositeString &other);
        CompositeString(const Char *str1, CommonSize size1, const Char *str2, CommonSize size2);
        CompositeString(const std::string_view &str1, const std::string_view &str2);
        CompositeString(CompositeString &&other) noexcept;
        ~CompositeString();

        void copy(const Char * str, CommonSize size);
        void copy(const CompositeString &other);
        void release();
        void reserve(CommonSize size);
        bool eq(const Char *other, CommonSize otherSize) const;
        bool lt(const Char *other, CommonSize otherSize) const;

        [[nodiscard]] const Char &at(CommonSize pos) const;
        [[nodiscard]] const Char &front() const;
        [[nodiscard]] const Char &back() const;
        const Char &operator[](CommonSize pos) const;
        
        //结尾没有 '\0'
        [[nodiscard]] const Char *c_str() const;
        [[nodiscard]] Char *data() const;
        [[nodiscard]] bool empty() const;
        [[nodiscard]] CommonSize size() const;
        [[nodiscard]] CommonSize unicodeSize() const;

        CompositeString &operator=(const CompositeString &other);

        //+ Char
        //+ const Char *
        CompositeString operator+(const CompositeString &other) const;

        bool operator==(const Char *ptr) const;
        bool operator<(const Char *ptr) const;

        bool operator==(const CompositeString &other) const;
        bool operator<(const CompositeString &other) const;

        bool operator==(const std::string_view &view) const;
        bool operator<(const std::string_view &view) const;
        
        friend std::ostream &operator<<(std::ostream &out, const CompositeString &str);
        
        //split
        //substr

        [[nodiscard]] HashType hashCode() const;
        [[nodiscard]] std::string toString() const;
        [[nodiscard]] std::string_view toStringView() const;
    };

}

namespace std {
    template<>
    struct hash<RexVM::CompositeString> {
        std::size_t operator()(const RexVM::CompositeString &str) const noexcept {
            return static_cast<std::size_t>(str.hashCode());
        }
    };
}

template<>
struct fmt::formatter<RexVM::CompositeString> : formatter<std::string_view> {
    template <typename FormatContext>
    auto format(const RexVM::CompositeString &myStr, FormatContext &ctx) const {
        return formatter<std::string_view>::format(myStr.toStringView(), ctx);
    }
};
#endif
