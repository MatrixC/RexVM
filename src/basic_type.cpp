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
               type == 'J' || type == 'C' || type == 'F' || type == 'D' || type == 'V';
    }

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

    cstring basicTypeClassName(BasicType type) {
        switch (type) {
            case BasicType::T_BOOLEAN:
                return "boolean";
            case BasicType::T_CHAR:
                return "char";
            case BasicType::T_FLOAT:
                return "float";
            case BasicType::T_DOUBLE:
                return "double";
            case BasicType::T_BYTE:
                return "byte";
            case BasicType::T_SHORT:
                return "short";
            case BasicType::T_INT:
                return "int";
            case BasicType::T_LONG:
                return "long";
            case BasicType::T_VOID:
                return "void";
            default:
                panic("type error");
                break;
        }
        return {};
    }

    const std::array<BasicType, 9> PRIMITIVE_TYPE_ARRAY = {
        BasicType::T_BOOLEAN,
        BasicType::T_CHAR,
        BasicType::T_FLOAT,
        BasicType::T_DOUBLE,
        BasicType::T_BYTE,
        BasicType::T_SHORT,
        BasicType::T_INT,
        BasicType::T_LONG,
        BasicType::T_VOID,
    };
}