#include "basic_type.hpp"
#include "exception.hpp"

namespace RexVM {
    const std::unordered_map<cstring, cstring> PRIMITIVE_TYPE_MAP{
            {"void",    "V"},
            {"boolean", "Z"},
            {"byte",    "B"},
            {"short",   "S"},
            {"int",     "I"},
            {"long",    "J"},
            {"char",    "C"},
            {"float",   "F"},
            {"double",  "D"},
    };

    const std::unordered_map<cstring, cstring> PRIMITIVE_TYPE_REVERSE_MAP{
            {"V",  "void"},
            {"Z",  "boolean"},
            {"B",  "byte"},
            {"S",  "short"},
            {"I",  "int"},
            {"J",  "long"},
            {"C",  "char"},
            {"F",  "float"},
            {"D",  "double"},
    };

    BasicType getBasicType(cchar type) {
        switch (type) {
            case 'V':
                return BasicType::T_VOID;
            case 'Z':
                return BasicType::T_BOOLEAN;
            case 'B':
                return BasicType::T_BYTE;
            case 'S':
                return BasicType::T_SHORT;
            case 'I':
                return BasicType::T_INT;
            case 'J':
                return BasicType::T_LONG;
            case 'C':
                return BasicType::T_CHAR;
            case 'F':
                return BasicType::T_FLOAT;
            case 'D':
                return BasicType::T_DOUBLE;
            default:
                panic(&"error type descriptor "[type]);
        }
        return BasicType::T_ILLEGAL;
    }

    bool isBasicType(cchar type) {
        return type == 'Z' || type == 'B' || type == 'S' || type == 'I' ||
               type == 'J' || type == 'C' || type == 'F' || type == 'D';
    }

    const std::unordered_map<cchar, BasicType> DESCRIPTOR_BASIC_TYPE_MAP{
            {'V', BasicType::T_VOID},
            {'Z', BasicType::T_BOOLEAN},
            {'B', BasicType::T_BYTE},
            {'S', BasicType::T_SHORT},
            {'I', BasicType::T_INT},
            {'J', BasicType::T_LONG},
            {'C', BasicType::T_CHAR},
            {'F', BasicType::T_FLOAT},
            {'D', BasicType::T_DOUBLE},
   };

    cstring typeArrayClassName(BasicType type) {
        switch (type) {
            case BasicType::T_BOOLEAN:
                return "[Z";
            case BasicType::T_CHAR:
                return "[C";
            case BasicType::T_FLOAT:
                return "[F";
            case BasicType::T_DOUBLE:
                return "[D";
            case BasicType::T_BYTE:
                return "[B";
            case BasicType::T_SHORT:
                return "[S";
            case BasicType::T_INT:
                return "[I";
            case BasicType::T_LONG:
                return "[J";

            default:
                panic("type error");
                break;
        }
        return {};
    }
}