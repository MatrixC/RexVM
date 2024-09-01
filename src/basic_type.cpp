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

    cchar getDescriptorByPrimitiveClassName(cview className) {
        if (className == "void") {
            return 'V';
        } else if (className == "boolean") {
            return 'Z';
        } else if (className == "byte") {
            return 'B';
        } else if (className == "short") {
            return 'S';
        } else if (className == "int") {
            return 'I';
        } else if (className == "long") {
            return 'J';
        } else if (className == "char") {
            return 'C';
        } else if (className == "float") {
            return 'F';
        } else if (className == "double") {
            return 'D';
        }
        panic("error class name");
        return 'V';
    }

    cstring getPrimitiveClassNameByDescriptor(cchar descriptor) {
        switch (descriptor) {
            case 'V':
                return "void";
            case 'Z':
                return "boolean";
            case 'B':
                return "byte";
            case 'S':
                return "short";
            case 'I':
                return "int";
            case 'J':
                return "long";
            case 'C':
                return "char";
            case 'F':
                return "float";
            case 'D':
                return "double";
            default:
                panic("error descriptor descriptor");
        }
        return {};
    }

    BasicType getBasicTypeByDescriptor(cchar descriptor) {
        switch (descriptor) {
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
                panic("error descriptor descriptor");
        }
        return BasicType::T_ILLEGAL;
    }

    BasicType getBasicTypeByTypeArrayClassName(const cstring &className) {
        if (className == "[Z") {
            return BasicType::T_BOOLEAN;
        } else if (className == "[B") {
            return BasicType::T_BYTE;
        } else if (className == "[S") {
            return BasicType::T_SHORT;
        } else if (className == "[I") {
            return BasicType::T_INT;
        } else if (className == "[C") {
            return BasicType::T_CHAR;
        } else if (className == "[J") {
            return BasicType::T_LONG;
        } else if (className == "[F") {
            return BasicType::T_FLOAT;
        } else if (className == "[D") {
            return BasicType::T_DOUBLE;
        }
        panic("getBasicTypeByTypeArrayClassName error: error className " + className);
        return BasicType::T_VOID;
    }

    bool isBasicType(cchar type) {
        return type == 'Z' || type == 'B' || type == 'S' || type == 'I' ||
               type == 'J' || type == 'C' || type == 'F' || type == 'D' || type == 'V';
    }

    cstring getTypeArrayClassNameByBasicType(BasicType type) {
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

    cstring getPrimitiveClassNameByBasicType(BasicType type) {
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

    bool isWideBasicType(BasicType type) {
        return type == BasicType::T_LONG || type == BasicType::T_DOUBLE;
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

    SlotTypeEnum getSlotTypeByPrimitiveClassName(const cstring &className) {
        SlotTypeEnum slotType;
        if (className == "boolean" || className == "byte" || className == "short" || className == "int" ||
            className == "char") {
            slotType = SlotTypeEnum::I4;
        } else if (className == "long") {
            slotType = SlotTypeEnum::I8;
        } else if (className == "float") {
            slotType = SlotTypeEnum::F4;
        } else if (className == "double") {
            slotType = SlotTypeEnum::F8;
        } else if (className == "void") {
            slotType = SlotTypeEnum::NONE;
        } else {
            slotType = SlotTypeEnum::REF;
        }

        return slotType;
    }

    size_t getElementSizeByBasicType(BasicType type) {
        switch (type) {
            case BasicType::T_BOOLEAN:
            case BasicType::T_BYTE:
                return sizeof(u1);
            case BasicType::T_CHAR:
                return sizeof(cchar_16);
            case BasicType::T_FLOAT:
                return sizeof(f4);
            case BasicType::T_DOUBLE:
                return sizeof(f8);
            case BasicType::T_SHORT:
                return sizeof(i2);
            case BasicType::T_INT:
                return sizeof(i4);
            case BasicType::T_LONG:
                return sizeof(i8);
            default:
                panic("type error");
                break;
        }
        return {};
    }
}