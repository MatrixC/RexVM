#ifndef BASIC_TYPE_HPP
#define BASIC_TYPE_HPP

#include <unordered_map>
#include <array>
#include "config.hpp"
#include "exception.hpp"

namespace RexVM {
    enum class BasicType : u1 {
        T_BOOLEAN = 4,
        T_CHAR = 5,
        T_FLOAT = 6,
        T_DOUBLE = 7,
        T_BYTE = 8,
        T_SHORT = 9,
        T_INT = 10,
        T_LONG = 11,
        T_OBJECT = 14,
        T_ARRAY = 13,
        T_VOID = 12,
        T_ADDRESS = 16,
        T_NARROWOOP = 251,
        T_METADATA = 252,
        T_NARROWKLASS = 253,
        T_CONFLICT = 254,
        T_ILLEGAL = 255,
    };

    extern const std::unordered_map<cstring, cstring> PRIMITIVE_TYPE_MAP;
    extern const std::unordered_map<cstring, cstring> PRIMITIVE_TYPE_REVERSE_MAP;
    extern const std::array<BasicType, 9> PRIMITIVE_TYPE_ARRAY;

    extern cstring typeArrayClassName(BasicType type);
    cstring basicTypeClassName(BasicType type);
    BasicType getBasicType(cchar type);
    bool isBasicType(cchar type);

}

#endif